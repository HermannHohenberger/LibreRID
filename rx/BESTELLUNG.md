# Bestellliste – RF-Frontend (Stand 2026-11-10)

Bequem beim Händler deiner Wahl; Preise Richtwerte ab China-Versand,
Alternative: deutsche Funkmodule-Händler (Voigted/ByteRenderer-Umfeld, Amazon)
mit kürzerer Lieferzeit.

## Phase 1: ELRS 2,4 GHz (SX1280)
**Klartext 2026-11-10:** EBYTE E220/E280 sind UART-Module mit eigener MCU –
kein Direktzugriff auf die SX1280-Register (kein CAD). E280-2G4T12S kann
trotzdem v0: dokumentierter RSSI-Modus (alle 100 ms RSSI) + 1-MHz-Kanalraster
→ RSSI-Sweep-Energiescanner. Für CAD (v1) braucht es ein nacktes SPI-SX1280-
Breakout (AliExpress „SX1280 SPI module"; Typ suchen, bevor bestellt wird).

| # | Artikel | Zweck | Stk | ca. € |
|---|---------|-------|----:|------:|
| 1 | **EBYTE E280-2G4T12S** (SX1280, UART, RSSI-Modus) | v0-Sweep-Scanner + Testsender-Paar | 2 | 24 |
| 2 | 2,4-GHz-Antenne IPEX/U.FL (falls Modulvariante ohne PCB-Funkantenne) | dito | 2 | 6 |
| 3 | USB-Seriell-Adapter 3,3 V (FT232/CP2102) | solo testen ohne ESP32 | 1 | 8 |
| 4 | Dupont-Kabel Buchstecker, 10 cm | Ansteuerung M0/M1/M2 + UART | 10 | — |

Hinweis: Amazon-Artikel B09CPDVJKP liefert laut Hermann an seine Adresse
(der „cannot be dispatched"-Hinweis auf der Seite betraf nur die dort gerade
eingestellte Versandadresse – im Warenkorb gegenprüfen).

**Später (v1, CAD):** nacktes SPI-SX1280-Breakout + Logikpegel-Check (3,3 V).

## Phase 1b: Crossfire 868 MHz (Experiment, später bestellbar)
| # | Artikel | Zweck | Stk | ca. € |
|---|---------|-------|----:|------:|
| 4 | RFM95W (Semtech SX1276, 868 MHz) | CAD-Test gegen Crossfire | 2 | 10 |
| 5 | 868/915-MHz-Antenne IPEX | dito | 2 | 6 |

## Nicht nötig (Vorhandenes reicht)
- Testsender: RadioMaster GX15 (ELRS) + Crossfire TX vorhanden
- Zielobjekte: ELRS-Kleinstdrohnen, Chupito vorhanden
- Empfänger-PC/Broker/ntfy: läuft alles auf dem VPS

## Gesamtsumme Phase 1: ca. 28 € (Phase 1b zusätzlich ca. 16 €)
