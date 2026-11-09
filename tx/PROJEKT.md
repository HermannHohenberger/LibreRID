# LibreRID-TX

**Status:** WARTUNG <!-- übernommen aus portfolio.md-Zeile LibreRID – bitte bestätigen/annehmen -->
**Bereich:** Privat
**Fokus (jetzt):** Läuft produktiv an der TBS-Chupito; Feature-Branch `codex/xiao-coded-phy-flight-tests` (XIAO Coded PHY + RF-Testprotokolle) noch nicht mit main gemergt
**Blockade:** -
**Zuletzt angefasst:** 2026-11-08 – Reorg in Repo-Unterordner tx/ (Inhalt unverändert)

## Wozu existiert das?

Eigenes OpenDroneID-Sendemodul (ESP32-S3) für RC-Modelle: liest INAV-Flugdaten per MSP
und sendet Remote-ID über BLE4 Legacy + BLE5 Long Range. Experimentell, kein
zertifiziertes Add-on.

## Wo ist was?

- Firmware: `src/` + `platformio.ini` (pio run)
- PRD: `LibreRID_PRD_v0.4/0.5.md` · Testprotokolle: `docs/`
- Prod: TBS-Chupito (SpeedyBee F405 Wing Mini, UART4)

## Wieder rein kommen

1. `README.md` (UART-Verdrahtungstabelle) → `pio run` → Flash via USB-C
2. Konfiguration: `src/configuration.h` (TEST-Profil beachten!)

## Offene Enden

- [ ] Feature-Branch reviewen und mergen (oder verwerfen)
- [ ] Portfolio-Zeile: altes `LibreRID` war WARTUNG – neuer Zustand = zwei Unterzeilen (tx/rx), Status je Strang setzt Hermann

## Entscheidungen (nur die teuren)

(siehe Root-README Reorg-Historie)
