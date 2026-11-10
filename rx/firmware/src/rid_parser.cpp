// Remote-ID-Parser: erkennt die drei in der Praxis vorkommenden BLE-Einbettungen
//  A) ASTM Direct:      AD 0x16, UUID 0xFFFA, dann [ctrl][ts][bis zu 4 Messages]
//     (nutzt auch unsere LibreRID-TX im Legacy-Strang)
//  B) ETSI gepackt:     AD 0x16, UUID 0xFFFA, dann [0x0D][counter][MessagePack]
//     (nutzt unsere LibreRID-TX im Coded-PHY-Strang)
//  C) ASTM mit Provider: AD 0x12, Payload beginnt 'U''T' ("UT")
//  D) ETSI im MfD:      AD 0xFF, Company-ID 0x0FF6C0 (EN 4709-002)
// Dekodierung per opendroneid-core-c (geteilt mit tx/).
#include "rid_parser.h"

extern "C" {
#include "opendroneid.h"
}

#include <string.h>
#include <stdio.h>

static uint32_t s_frames = 0;
uint32_t rid_parser_frame_count() { return s_frames; }

void RidFix::reset() {
    memset(this, 0, sizeof(*this));
}

static void copy_str(char *dst, size_t cap, const char *src, size_t srclen) {
    size_t n = srclen < cap - 1 ? srclen : cap - 1;
    memcpy(dst, src, n);
    dst[n] = '\0';
}

// Ein dekodiertes UAS-Abbild in einen RidFix mergen.
static void merge_uas(RidFix &out, ODID_UAS_Data &uas) {
    if (uas.BasicIDValid[0]) {
        copy_str(out.id, sizeof(out.id), uas.BasicID[0].UASID,
                 strnlen(uas.BasicID[0].UASID, ODID_ID_SIZE));
        out.id_type = (uint8_t)uas.BasicID[0].IDType;
        out.has_id = true;
    }
    if (uas.LocationValid) {
        out.lat = uas.Location.Latitude;
        out.lon = uas.Location.Longitude;
        out.alt = uas.Location.AltitudeBaro;
        if (out.alt == 0) out.alt = uas.Location.AltitudeGeo;
        out.speed = uas.Location.SpeedHorizontal;
        out.direction = uas.Location.Direction;
        memcpy(&out.status, &uas.Location.Status, 1);  // Bitfeld robust übernehmen
        out.has_location = true;
    }
    if (uas.OperatorIDValid) {
        copy_str(out.operator_id, sizeof(out.operator_id),
                 uas.OperatorID.OperatorId,
                 strnlen(uas.OperatorID.OperatorId, ODID_ID_SIZE));
        out.has_operator = true;
    }
    // Hinweis: diese opendroneid-Version führt kein OperatorCountry im System.
    (void)0;
}

// ASTM-Paketierung: [ctrl][timestamp(4)][Messages a 25 Byte]
static bool decode_astm_packed(const uint8_t *p, uint16_t avail, RidFix &out) {
    if (avail < 6) return false;
    uint8_t count = (p[0] >> 4) & 0x0F;
    uint16_t room = (avail - 6) / ODID_MESSAGE_SIZE;
    if (count == 0 || room == 0) return false;
    if (count > room) count = (uint8_t)room;
    if (count > 4) count = 4;

    ODID_UAS_Data uas;
    memset(&uas, 0, sizeof(uas));
    bool any = false;
    for (uint8_t i = 0; i < count; i++) {
        const uint8_t *msg = p + 6 + i * ODID_MESSAGE_SIZE;
        uint8_t id = msg[0] & 0x0F;
        switch (id) {
            case 0:
                if (decodeBasicIDMessage(&uas.BasicID[0],
                        (const ODID_BasicID_encoded *)msg) == ODID_SUCCESS)
                    uas.BasicIDValid[0] = 1, any = true;
                break;
            case 1:
                if (decodeLocationMessage(&uas.Location,
                        (const ODID_Location_encoded *)msg) == ODID_SUCCESS)
                    uas.LocationValid = 1, any = true;
                break;
            case 4:
                if (decodeSystemMessage(&uas.System,
                        (const ODID_System_encoded *)msg) == ODID_SUCCESS)
                    uas.SystemValid = 1, any = true;
                break;
            case 5:
                if (decodeOperatorIDMessage(&uas.OperatorID,
                        (const ODID_OperatorID_encoded *)msg) == ODID_SUCCESS)
                    uas.OperatorIDValid = 1, any = true;
                break;
            default:
                break;  // Auth/SelfID: für v0 nicht ausgewertet
        }
    }
    if (!any) return false;
    s_frames++;
    merge_uas(out, uas);
    return true;
}

// ETSI-MessagePack (vom Encoder, wie TX ihn sendet)
static bool decode_etsi_pack(const uint8_t *p, uint16_t avail, RidFix &out) {
    if (avail < 6) return false;
    ODID_UAS_Data uas;
    memset(&uas, 0, sizeof(uas));
    if (decodeMessagePack(&uas, (const ODID_MessagePack_encoded *)p) !=
        ODID_SUCCESS)
        return false;
    if (!uas.BasicIDValid[0] && !uas.LocationValid && !uas.OperatorIDValid &&
        !uas.SystemValid)
        return false;
    s_frames++;
    merge_uas(out, uas);
    return true;
}

bool parse_adv_report(const uint8_t *data, uint16_t len,
                      const uint8_t addr[6], uint8_t addr_type, int8_t rssi,
                      uint8_t phy_primary, uint8_t phy_secondary, RidFix &out) {
    out.reset();
    snprintf(out.mac, sizeof(out.mac), "%02x:%02x:%02x:%02x:%02x:%02x",
             addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
    out.rssi = rssi;
    out.phy_primary = phy_primary;
    out.phy_secondary = phy_secondary;
    (void)addr_type;

    bool hit = false;
    uint16_t i = 0;
    while (i + 1 < len) {
        uint8_t ad_len = data[i];
        if (ad_len == 0 || i + 1 + ad_len > len) break;
        uint8_t type = data[i + 1];
        const uint8_t *p = data + i + 2;
        uint16_t plen = ad_len - 1;

        if (type == 0x16 && plen >= 6 && p[0] == 0xFA && p[1] == 0xFF) {
            if (p[2] == 0x0D)
                hit |= decode_etsi_pack(p + 4, plen - 4, out);   // Variante B
            else
                hit |= decode_astm_packed(p + 2, plen - 2, out); // Variante A
        } else if (type == 0x12 && plen >= 8 && p[0] == 'U' && p[1] == 'T') {
            hit |= decode_astm_packed(p + 2, plen - 2, out);     // Variante C
        } else if (type == 0xFF && plen >= 8) {
            uint32_t company = p[0] | (p[1] << 8) | (p[2] << 16);
            if (company == 0x0FF6C0) {                            // Variante D
                if (p[3] == 0x12)
                    hit |= decode_astm_packed(p + 4, plen - 4, out);
                else if (p[3] == 'U' && p[4] == 'T')
                    hit |= decode_astm_packed(p + 5, plen - 5, out);
            }
        }
        i += 1 + ad_len;
    }
    return hit;
}
