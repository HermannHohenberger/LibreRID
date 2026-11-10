// LibreRID-RX Konfiguration – hier SSID/Passwort/MQTT eintragen.
// Hinweis Feldeinsatz: Der RX verbindet sich in den Handy-Hotspot (Betriebsfall A).
// Zu Hause: Heim-WLAN. Es werden bis zu drei Netze der Reihe nach probiert.
#pragma once

#define RX_NAME "s3-01"  // Empfänger-Kennung in allen MQTT-Events

// ---- Netze (Reihenfolge = Priorität) ----
static const char *WIFI_SSIDS[] = {
    "HEIMWLAN-SSID",      // zu Hause
    "HANDY-HOTSPOT-SSID", // Flugplatz / Feld
};
static const char *WIFI_PASSES[] = {
    "HEIMWLAN-PASSWORT",
    "HOTSPOT-PASSWORT",
};
constexpr int WIFI_NETWORKS =
    sizeof(WIFI_SSIDS) / sizeof(WIFI_SSIDS[0]);

// ---- MQTT ----
#define MQTT_HOST "192.168.4.100" // Mosquitto auf dem HomeLab
#define MQTT_PORT 1883
#define MQTT_USER ""              // leer = anonym
#define MQTT_PASS ""
#define MQTT_TOPIC_PREFIX "librerid/" // Thema: librerid/<RX_NAME>

// ---- Verhalten ----
#define PUBLISH_FIX_INTERVAL_MS 1000  // Lage-Fix je Drohne max. 1x/s
#define TABLE_EXPIRY_MS (60 * 1000)   // Eintrag gilt als verschwunden
#define HEARTBEAT_INTERVAL_MS (30 * 1000)
#define SCAN_REPORT_INTERVAL_MS (10 * 1000) // serieller Status, wenn Ruhe ist
