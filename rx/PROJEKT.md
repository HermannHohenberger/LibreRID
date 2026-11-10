# LibreRID-RX (Knoten v0)

**Status:** EXPERIMENT
**Bereich:** Privat (Entwicklung)
**Fokus (jetzt):** v0 auf Brett geflasht & live: Dual-PHY-Scan läuft (cfg_mask 0x03), JSON-Pipeline seriell bewiesen. Nächste Schritte: config.h befüllen (WLAN/MQTT) → MQTT-Strecke HomeLab → Feldtest mit Chupito-TX am Modellflugplatz.
**Blockade:** -
**Zuletzt angefasst:** 2026-11-08 – E1 entschieden (S3 vorhanden), Meshtastic-Datenpfad konkretisiert

## Wozu existiert das?

Passiver Remote-ID-Empfänger als Keimzelle des Groundstack: erst eigene Chupito
(LibreRID-TX) sehen und Überflüge des eigenen Grundstücks protokollieren
(„Liegenschaftswache"), später Mesh und mehr Erkennungsstufen.

## Wo ist was?

- Konzept v0: `docs/KONZEPTION.md` (Architektur, Hardware-Matrix, Demo-Plan, Rechtsrahmen)
- Firmware: wird `firmware/` (ESP-IDF, BLE-Scan + WiFi-Monitor + opendroneid-core-c)
- Server/Dashboard: wird `server/` (MQTT-Ingest, SQLite, Leaflet)

## Wieder rein kommen

1. `docs/KONZEPTION.md` lesen (Entscheidungspunkte E1–E3 oben)
2. TX als Testsender: `../tx/` (README verdrahtet Chupito)
3. Referenz-Geiger zum Gegenprüfen: OpenDroneID `receiver-android` aufs Handy

## Offene Enden

- [x] E1: ESP32-Typ im Regal geklärt – gemessen 2026-11-10: ESP32-S3 rev v0.2, 8 MB PSRAM, BT5 LE (Pfad B)
- [ ] E3: Dashboard-Stack wählen (Empfehlung Python)
- [ ] MQTT-Broker-Topic-Design (`rid/<empfaenger>` auf dem HomeLab)
- [ ] Nach go: Portieren aus dem Startup-Trichter in dieses Repo ist erledigt – dieses hier ist fortan der Produktordner

## Entscheidungen (nur die teuren)

- 2026-11-08 – Ein Repo „LibreRID", zwei Stränge tx/ und rx/ – geteiltes Protokoll, geteilte Story; Ausgliederung von rx bleibt möglich.
- 2026-11-08 – RX wohn_t im LibreRID-Repo (nicht neues Projekt „RidNode") – der Trichter bleibt dünn.
