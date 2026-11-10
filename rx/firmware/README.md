# LibreRID-RX Firmware v0

Bodensender-Empfänger für Drone Remote ID über BLE:
**Dual-Phy-Scan (1M Legacy + Coded/Long Range)** über die Bluedroid-Extended-Scan-API,
Dekodierung mit `opendroneid-core-c` (geteilt mit `tx/`), Ausgaben als
**serielle JSON-Zeilen** und **MQTT** pro Empfänger.

## Build & Flash

```bash
cd rx/firmware
/opt/homebrew/bin/pio run                 # bauen
/opt/homebrew/bin/pio run -t upload       # flashen
/opt/homebrew/bin/pio device monitor      # JSON-Ausgabe ansehen
```

Falls der Port keine Bootloader-Antwort gibt (Board läuft mit User-Firmware):
Taster **BOOT halten, RST kurz drücken, BOOT loslassen**, dann erneut flashen.
Das Brett hat beim Ausliefern eine stumme TinyUSB-Demo-Firmware.

## Konfiguration

Alles in `src/config.h`: Name, bis zu zwei WLANs (Heim + Handy-Hotspot),
MQTT-Broker. Passwort-Klartext nur lokal lassen – nicht committen!
(Abmachung: Secrets bleiben außerhalb des Repos oder werden vor Push entfernt.)

## Ausgabeprotokoll (JSON, eine Zeile je Event)

| t      | Bedeutung                                             |
|--------|-------------------------------------------------------|
| `seen` | neue Drohne (Basic ID)                                 |
| `fix`  | Lagefix, je Drohne max. 1/s (lat/lon/alt/rssi/phy)     |
| `hb`   | Heartbeat alle 30 s (Tabelle, Zähler, Status)          |

MQTT-Thema: `librerid/<RX_NAME>`. Dieselben Zeilen laufen seriell mit –
damit ist die Kette auch ohne Netz beweisbar. `phys` zeigt die HCI-Phy-Rohwerte:
hier sehen wir erstmals, wer auf Coded PHY (Long Range) sendet.

## Selbsttests

1. **Heimstation (Ernstfall):** Konfig eintragen, flashern, monitor laufen lassen –
   über Stadtgebiet sendet regulärer Drohnenverkehr (DJI & Co. senden RID Pflicht).
2. **Prüfstand (kontrolliert):** Chupito mit LibreRID-TX in Reichweite bringen –
   Legacy-Strang (1M) und Coded-Strang müssen als `phys`-Wert sichtbar werden.
3. **Hotspot-Fall:** Handy-Tethering an, SSID in config, am Feldrand:
   `fix`-Zeilen erscheinen, MQTT verbindet übers Mobilnetz.

## Grenzen v0 (bewusst)

- Fragmentierte Extended-Ads (data_status != complete) werden verworfen –
  reale RID-Packs passen in ein 251-Byte-Event.
- SelfID/Auth-Nachrichten werden dekodiert, aber nicht gemeldet.
- Kein Flash-Speicher der Tabelle (Neustand nach Reboot) – v1 mit NVS/PSRAM-Ring.
