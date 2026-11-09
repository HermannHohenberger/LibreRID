# LibreRID-RX – Empfänger

Passiver Remote-ID-Empfänger (ESP32, BLE + WiFi) mit MQTT-Ausgang – Keimzelle des
Groundstack / der „Liegenschaftswache".

**Aktueller Stand:** Konzeptionsphase. Alles Wesentliche in
**[`docs/KONZEPTION.md`](docs/KONZEPTION.md)** – Architektur-Skizze,
ESP32-Chip-Matrix (BLE5-Long-Range-Entscheidung), Referenz-Fundus
(opendroneid-core-c, ArduRemoteID, Drone-Detector-V2, DaringDroneDetector),
Demo-Plan und Rechtsrahmen.

Testsender: Hausintern `../tx/` (LibreRID auf Chupito, sendet BLE4 Legacy + BLE5 LR).
