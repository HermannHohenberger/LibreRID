// RidFix: ein interpretierter Remote-ID-Funkfund.
#pragma once
#include <stdint.h>
#include <stddef.h>

struct RidFix {
    char mac[18];           // "aa:bb:cc:dd:ee:ff" des Senders
    char id[24];            // Basic ID (UASID), sonst ""
    uint8_t id_type;        // ODID IDType (roh)
    char operator_id[24];   // Operator-ID (Fern-ID), sonst ""
    uint16_t country;       // Betreiberland (ISO-Nummerncode, roh)
    double lat, lon;        // Grad
    double alt;             // Barometrische Höhe [m]; bei 0 -> Geo-Höhe
    float speed;            // m/s horizontal
    float direction;        // Grad über Grund
    uint8_t status;         // ODID UAS-Status (roh)
    int8_t rssi;
    uint8_t phy_primary;    // HCI-Rohwerte, damit erkennen wir 1M vs. Coded
    uint8_t phy_secondary;
    bool has_id, has_location, has_operator;

    RidFix() { reset(); }
    void reset();
};

// Liefert true, wenn in der Advertisement-Nutzlast eine Remote-ID-Struktur
// erkannt und mindestens ein Teilpaket dekodiert werden konnte.
// data/len = rohe AD-Strukturen aus dem Scan-Report.
bool parse_adv_report(const uint8_t *data, uint16_t len,
                      const uint8_t addr[6], uint8_t addr_type, int8_t rssi,
                      uint8_t phy_primary, uint8_t phy_secondary, RidFix &out);

// Anzahl geparster RID-Frame (Diagnose)
uint32_t rid_parser_frame_count();
