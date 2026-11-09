# LibreRID – das offene Remote-ID-Paar

OpenDroneID für ESP32, zwei Stränge, ein Protokoll:

| Strang | Was es ist | Status |
|---|---|---|
| **[tx/](tx/)** | **Sender:** broadcasts OpenDroneID (BLE 4 Legacy + BLE 5 Long Range) from INAV flight data – produktiv an der TBS-Chupito (ESP32-S3) | läuft, [Steuerblatt](tx/PROJEKT.md) |
| **[rx/](rx/)** | **Empfänger:** scans BLE/WiFi Remote-ID, forwards to MQTT – Basis der „Liegenschaftswache" | in Konzeption, [Steuerblatt](rx/PROJEKT.md) |

TX + RX zusammen sind die Keimzelle des **Groundstack**: offener Knoten (Empfänger) →
Mesh → Dokumentation/Alarmierung. Sender und Empfänger teilen die
`opendroneid-core-c`-Dekodierung und die ASTM-F3411-Message-Formate.

## Demo-Pfad (v0)

Chupito mit TX → rx-Knoten empfängt → MQTT → Live-Karte mit Grundstückspolygon →
Überflug-Protokoll. Konzeptdetails: [`rx/docs/KONZEPTION.md`](rx/docs/KONZEPTION.md)

## Rechtsstatus

Experimentier-Firmware, **kein zertifiziertes EU-Direct-Remote-ID-Add-on**.
TX nutzt TEST-Profil – Testbetrieb nur auf eigenem Gelände. RX ist passiver Empfang
(in DE für jedermann zulässig).

## Reorg-Historie

Bis 2026-11-08 lag die TX-Firmware im Repo-Root; sie lebt unverändert unter `tx/`
(Git-Rename, Historie erhalten).
