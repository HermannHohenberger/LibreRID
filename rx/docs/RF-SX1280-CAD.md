# RF-Frontend v0: SX1280-CAD-Scanner (nicht-kooperative RC-Erkennung)

Beschlossen 2026-11-10 als nächstes rx-Experiment. Idee von Hermann:
„ELRS-Empfänger ans ESP32 zur Signalerkennung" → korrekt umgesetzt: **nackter
Semtech SX1280 per SPI**, kein ELRS-Modul (dessen Firmware ist geschlossen und
liefert nur eigenen CRSF-Verkehr nach bekanntem Binding).

## Warum ELRS/LoRa der Ziel-Signaltyp ist
- ELRS ist die Standard-Funkstrecke bei DIY-/Aggressor-FPV-Drohnen.
- 2,4-GHz-Variante: LoRa-Modulation (BW ~0,8 MHz, SF6–SF12), FHSS über 80
  Kanäle von ca. 2400,4–2483,5 MHz (~1,05 MHz Abstand). Kein RID, keine
  Kennung – genau die Lücke, die der Markt (Dronetag & Co.) nicht füllt.
-SX1280 beherrscht exakt diese Strecke und hat zwei Detektoren ohne
  Decodiervorwissen:
  1. **CAD** (Channel Activity Detection): LoRa-Aktivität auf einem Kanal in
     ~200 µs erkennbar.
  2. **RSSI** pro Kanal: wiederholt über das Kanalraster gemessen →
     Hopping-Muster rekonstruierbar (Kern-Differenzierung).

## Hardware (Bestellliste)
- 2x Semtech SX1280 Modul, z. B. **E220-400M22S** (~10 €/St.) – SPI, 3,3 V,
  IPEX-Anschluss; eines als Lauscher am RX, eines ggf. als kontrollierter
  Testsender (mit Arduino-Sketch, LoRa-PRBS auf festem Kanal).
- 2x 2,4-GHz-Antenne IPEX/U.FL (oder Modul-Antennenbuchse beachten).
- Optional später: SX1262 (Sub-GHz ELRS 868/915) für dieselbe Rolle in EU-
  Low-Band; SX1280 bleibt Priorität 1 (Aggressoren nutzen 2,4G).

## Testaufbau
- Chupito mit ELRS-Fernsteuerung = realistischer Testsender (Link-Details
  egal – es geht um Präsenz, nicht um Inhalt).
- SX1280 am ESP32-S3-DevKitC (SPI vspi2/VSPI, 4 Drähte + CS + RST/BUSY/IRQ).
- Parallel bleibt der BLE-RID-Scan an (Dual-Sensorik auf einem Brett).

## Firmware v0 (Lernpfad, ~2 Abende)
1. SX1280-Treiber: SPI-Reset, Pause-Modus, CAD-Setup (LoRa, SF6–12 Autodetect).
2. Scan-Zyklus: Kanalraster 2400,4–2483,5 MHz Schritt ~1 MHz → je Kanal
   CAD + RSSI; Treffer mit Zeitstempel in Ringpuffer.
3. Aggregation: ≥3 Treffer in <2 s auf wechselnden Kanälen = „FHSS-Kandidat".
4. MQTT (bestehende Kette!): neues Event
   `{"t":"rf","ev":"fhss","ch":[...],"rssi":-87,"dur_ms":1500}` →
   ntfy-Push „möglicher RC-Link erkannt".
5. Später (v1): Hopping-Fingerabgleich (Frequenzfolge + Dwell-Zeit) als
   Protokoll-Vorhersage (ELRS vs. FrSky vs. Walkera).

## Grenzen (ehrlich)
- FLRC/FSK-Raten (High-Rate-ELRS) haben keinen LoRa-Präambel-CAD-Finger →
  nur Energie-RSSI; CAD läuft ins Leere. Daher v0 als „LoRa-RC-Erkennung".
- WiFi-APs belegen Teile des Bereichs (Kanal 1–13 überlappen das ELRS-Raster)
  → Filterregel: permanenter Schmalband-CAD-Treffer auf WiFi-Mitte = Ausschluss.
- 500-Hz-Hopping kann der S3 nicht kanalweise folgen; v0 erkennt Aktivität
  mit Verzögerung/Statistik, nicht jeden Sprung.
- Kein Richtungsfinden (dafür später 2 Antennen/TDoA-Experiment).

## Nächste Schritte
- [ ] Module bestellen (2x E220-400M22S + Antennen)
- [ ] Chupito-Funkstrecke klären (ELRS? Frequenzband?) – bestimmt den Test
- [ ] SPI-Bring-Up + CAD-Echow auf Serial
- [ ] MQTT-Event `rf` + Brückenerweiterung in `rx/tools/vps_ntfy_bridge.py`
