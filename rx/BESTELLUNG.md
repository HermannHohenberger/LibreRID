# Bestellliste – RF-Frontend (Stand 2026-11-10)

Bequem beim Händler deiner Wahl; Preise Richtwerte ab China-Versand,
Alternative: deutsche Funkmodule-Händler (Voigted/ByteRenderer-Umfeld, Amazon)
mit kürzerer Lieferzeit.

## Phase 1: ELRS 2,4 GHz (SX1280)
| # | Artikel | Zweck | Stk | ca. € |
|---|---------|-------|----:|------:|
| 1 | E220-400M22S (Semtech SX1280, SPI, 3,3 V, IPEX) | Lauscher am ESP32-S3 | 2 | 22 |
| 2 | 2,4-GHz-Antenne IPEX/U.FL (2 dBi reicht) | dito | 2 | 6 |
| 3 | Dupont-Kabel Buchstecker, 10 cm | SPI zum DevKitC | 10 | — |

Alternative zu 1: jedes nackte SX1280-Breakout (Ausklaus/SX1280-Modul);
**kein** ELRS-RX-/TX-Modul (geschlossene Firmware, siehe RF-SX1280-CAD.md).

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
