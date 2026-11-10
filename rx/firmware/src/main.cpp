// LibreRID-RX v0
// BLE-Sniffer (Dual-PHY: 1M Legacy + Coded/LongRange) für Remote ID,
// Ausgaben: serielle JSON-Zeilen + MQTT (librerid/<RX_NAME>).
// Pfade:  Heim-WLAN -> HomeLab-Mosquitto    |    Handy-Hotspot -> VPN -> ntfy-Kette
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BLEDevice.h>
#include <PubSubClient.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_gap_ble_api.h"

#include "config.h"
#include "rid_parser.h"
#include "drone_table.h"

// ---- Funkstrecke -> Verarbeitungsschlange -------------------------------
struct AdvReportLite {
    uint8_t data[251];
    uint16_t len;
    int8_t rssi;
    uint8_t addr[6];
    uint8_t addr_type;
    uint8_t phy_p, phy_s;
};

static QueueHandle_t s_queue = nullptr;
static uint32_t s_reports = 0, s_dropped = 0;

static void gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
    if (event != ESP_GAP_BLE_EXT_ADV_REPORT_EVT) return;
    const esp_ble_gap_ext_adv_reprot_t &r = param->ext_adv_report.params;
    if (r.adv_data_len == 0 || r.adv_data_len > sizeof(AdvReportLite::data))
        return;
    // Vorfilter: nur Frames, die RID-Datentypen enthalten können.
    bool maybe = false;
    for (uint16_t i = 0; i + 1 < r.adv_data_len; ) {
        uint8_t ad_len = r.adv_data[i];
        if (ad_len == 0 || i + 1 + ad_len > r.adv_data_len) break;
        uint8_t t = r.adv_data[i + 1];
        if (t == 0x16 || t == 0x12 || t == 0xFF) { maybe = true; break; }
        i += 1 + ad_len;
    }
    if (!maybe) return;

    AdvReportLite item;
    memcpy(item.data, r.adv_data, r.adv_data_len);
    item.len = r.adv_data_len;
    item.rssi = r.rssi;
    memcpy(item.addr, r.addr, 6);
    item.addr_type = r.addr_type;
    item.phy_p = (uint8_t)r.primary_phy;
    item.phy_s = (uint8_t)r.secondly_phy;
    if (xQueueSend(s_queue, &item, 0) == pdTRUE) s_reports++;
    else s_dropped++;
}

static void start_dual_phy_scan() {
    BLEDevice::init("librerid-rx");
    esp_ble_gap_register_callback(gap_cb);

    esp_ble_ext_scan_params_t p = {};
    p.own_addr_type = BLE_ADDR_TYPE_PUBLIC;
    p.filter_policy = BLE_SCAN_FILTER_ALLOW_ALL;
    p.scan_duplicate = BLE_SCAN_DUPLICATE_ENABLE;
    // 0x01 = 1M-Phy, 0x02 = Coded Phy – ohne beide Bits gilt „kein Phy" und
    // der Controller lehnt mit Invalid Parameters (0x12) ab.
    p.cfg_mask = 0x01 | 0x02;
    p.uncoded_cfg.scan_type = BLE_SCAN_TYPE_PASSIVE;
    p.uncoded_cfg.scan_interval = 0x0060;  // 60 ms
    p.uncoded_cfg.scan_window = 0x0030;    // 30 ms
    p.coded_cfg.scan_type = BLE_SCAN_TYPE_PASSIVE;
    p.coded_cfg.scan_interval = 0x00A0;    // 100 ms
    p.coded_cfg.scan_window = 0x0050;      // 50 ms

    esp_err_t e1 = esp_ble_gap_set_ext_scan_params(&p);
    // Dauer 0 ist laut API ungültig (Range 0x0001..0xFFFF in 10-ms-Schritten).
    // Wir scannen 655 s und setzen alle 10 Minuten neu an (service_scan()).
    esp_err_t e2 = esp_ble_gap_start_ext_scan(0xFFFF, 0);
    Serial.printf("scan: set_params=%d start=%d\n", e1, e2);
}

static void rescan_if_due() {
    static uint32_t last_start = 0;
    uint32_t now = millis();
    if (last_start == 0) { last_start = now; return; }
    if (now - last_start > 600000) {  // alle 10 min neu starten (Puffer-Frische)
        last_start = now;
        esp_ble_gap_start_ext_scan(0xFFFF, 0);
    }
}

// ---- Netzwerk -----------------------------------------------------------
static WiFiClient s_wifi_client;
static PubSubClient s_mqtt(s_wifi_client);
static int s_net = 0;
static uint32_t s_net_try_ms = 0, s_mqtt_try_ms = 0;

static void service_network() {
    if (WiFi.status() == WL_CONNECTED) return;
    uint32_t now = millis();
    if (now - s_net_try_ms < 10000) return;
    Serial.printf("wifi: versuche '%s' (%d/%d)\n", WIFI_SSIDS[s_net], s_net + 1,
                  WIFI_NETWORKS);
    WiFi.disconnect();
    WiFi.begin(WIFI_SSIDS[s_net], WIFI_PASSES[s_net]);
    s_net = (s_net + 1) % WIFI_NETWORKS;
    s_net_try_ms = now;
}

static void service_mqtt() {
    if (WiFi.status() != WL_CONNECTED) return;
    if (s_mqtt.connected()) { s_mqtt.loop(); return; }
    uint32_t now = millis();
    if (now - s_mqtt_try_ms < 5000) return;
    s_mqtt_try_ms = now;
    s_mqtt.setServer(MQTT_HOST, MQTT_PORT);
    s_mqtt.setBufferSize(1024);
    if (s_mqtt.connect(RX_NAME, MQTT_USER[0] ? MQTT_USER : nullptr,
                       MQTT_PASS[0] ? MQTT_PASS : nullptr))
        Serial.println("mqtt: verbunden");
}

// ---- Publikation --------------------------------------------------------
static void emit(const char *json) {
    Serial.println(json);  // immer: serielle JSON-Zeile (Beweis/Bench)
    if (s_mqtt.connected())
        s_mqtt.publish(MQTT_TOPIC_PREFIX RX_NAME, json);
}

static void publish_seen(const DroneEntry &e) {
    char buf[320];
    snprintf(buf, sizeof(buf),
             "{\"t\":\"seen\",\"rx\":\"%s\",\"id\":\"%s\",\"mac\":\"%s\","
             "\"otype\":%u,\"ms\":%lu}",
             RX_NAME, e.last.id, e.last.mac, e.last.id_type,
             (unsigned long)e.first_ms);
    emit(buf);
}

static void publish_fix(const DroneEntry &e) {
    char buf[480];
    snprintf(buf, sizeof(buf),
             "{\"t\":\"fix\",\"rx\":\"%s\",\"id\":\"%s\",\"mac\":\"%s\","
             "\"lat\":%.7f,\"lon\":%.7f,\"alt\":%.1f,\"spd\":%.1f,"
             "\"dir\":%.0f,\"st\":%u,\"rssi\":%d,\"phys\":[%u,%u]"
             "%s%s%s,\"ms\":%lu}",
             RX_NAME, e.last.id, e.last.mac, e.last.lat, e.last.lon, e.last.alt,
             e.last.speed, e.last.direction, e.last.status, e.last.rssi,
             e.last.phy_primary, e.last.phy_secondary,
             e.last.has_operator ? ",\"op\":\"" : "",
             e.last.has_operator ? e.last.operator_id : "",
             e.last.has_operator ? "\"" : "",
             (unsigned long)e.last_ms);
    emit(buf);
}

static void publish_heartbeat() {
    char buf[160];
    snprintf(buf, sizeof(buf),
             "{\"t\":\"hb\",\"rx\":\"%s\",\"n\":%d,\"up\":%lu,"
             "\"frames\":%lu,\"reports\":%lu,\"dropped\":%lu,\"wifi\":%d}",
             RX_NAME, table_count(), (unsigned long)(millis() / 1000),
             (unsigned long)rid_parser_frame_count(),
             (unsigned long)s_reports, (unsigned long)s_dropped,
             WiFi.status() == WL_CONNECTED);
    emit(buf);
}

// ---- Hauptprogramm ------------------------------------------------------
void setup() {
    Serial.begin(115200);
    delay(1500);
    Serial.println();
    Serial.printf("LibreRID-RX v0  name=%s\n", RX_NAME);

    s_queue = xQueueCreate(16, sizeof(AdvReportLite));
    table_init();
    start_dual_phy_scan();

    // Wichtig: WiFi-Modem-Sleep AN lassen – sonst abort() des Koexistenz-Stacks,
    // solange Bluetooth aktiv ist (Fehlerbild: "Should enable WiFi modem sleep").
    service_network();
}

void loop() {
    uint32_t now = millis();
    static uint32_t last_expire = 0, last_hb = 0, last_diag = 0;

    service_network();
    service_mqtt();
    rescan_if_due();

    // Funkfunde abarbeiten
    AdvReportLite item;
    int budget = 32;
    while (budget-- > 0 && xQueueReceive(s_queue, &item, 0) == pdTRUE) {
        RidFix fix;
        if (!parse_adv_report(item.data, item.len, item.addr, item.addr_type,
                              item.rssi, item.phy_p, item.phy_s, fix))
            continue;
        bool fresh = table_update(fix);
        DroneEntry *e = nullptr;
        for (int i = 0; (e = const_cast<DroneEntry *>(table_get(i))); i++) {
            if (strcmp(e->last.mac, fix.mac) == 0) break;
        }
        if (!e) continue;
        if (!e->id_announced && e->last.has_id) {
            e->id_announced = true;
            publish_seen(*e);
        }
        if (e->last.has_location &&
            now - e->last_pub_ms >= PUBLISH_FIX_INTERVAL_MS) {
            e->last_pub_ms = now;
            publish_fix(*e);
        }
    }

    if (now - last_expire > 1000) {
        last_expire = now;
        table_expire(now);
    }
    if (now - last_hb > HEARTBEAT_INTERVAL_MS) {
        last_hb = now;
        publish_heartbeat();
    }
    if (now - last_diag > SCAN_REPORT_INTERVAL_MS) {
        last_diag = now;
        Serial.printf("[di] reports=%lu dropped=%lu frames=%lu drones=%d rssi-free-heap=%lu\n",
                      (unsigned long)s_reports, (unsigned long)s_dropped,
                      (unsigned long)rid_parser_frame_count(), table_count(),
                      (unsigned long)ESP.getFreeHeap());
    }
    delay(2);
}
