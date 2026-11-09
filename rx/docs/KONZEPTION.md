# Knoten v1 – Konzeption RID-Empfänger (Liegenschaftswache-Demo)

Stand: 2026-11-08 · Status: Konzept v0 · Arbeitsname: **LibreRID-RX (Knoten v0)**
Zuhause: `CLI-Projekte/LibreRID/rx/` (ein Repo, zwei Stränge – Entscheidung 2026-11-08,
siehe `rx/PROJEKT.md`).

## 1. Zweck v0

Beweisen, nicht optimieren: Ein ESP32-Empfänger am eigenen Grundstück sieht die
**Remote-ID-Signatur der eigenen Chupito (LibreRID-Sender)**, zeichnet Flugbahnen auf,
prüft sie gegen ein **eingzeichnetes Grundstückspolygon** und erzeugt ein
**Überflug-Protokoll** – die Liegenschaftswache als 20-Sekunden-Demo.

**In v0 drin:** BLE/WiFi-RID-Empfang, Live-Karte, Polygon, Event-Log.
**Bewusst NICHT in v0:** Mesh, FHSS/Fingerprint, Akustik, Outdoor-Gehäuse, GNSS am
Empfänger, RED-Zertifizierung, Multi-User. Alles später, alles Stufen desselben Knotens.

## 2. Hardware

### 2.1 Empfänger-Platine – die eine ehrliche Einschränkung

| Chip | BLE4 Legacy RID | **BLE5 Long Range (Coded PHY)** | WiFi-RID (DJI/ASTM-NAN) | Hausnummer |
|---|---|---|---|---|
| ESP32 klassisch (WROOM) | ✅ | **❌ (nur BLE 4.2)** | ✅ (Monitor-Mode) | – |
| ESP32-C3 / -S3 | ✅ | ✅ | ✅ | 6–12 € |
| ESP32-C6 | ✅ | ✅ | ✅ (+ 5-GHz-WiFi) | ~10 € |

Der EU-Standard-Transport (EN 4709-002) ist **Bluetooth 5 Long Range** – den sieht ein
klassischer ESP32 nicht. Konsequenz:
- **Glücksfall:** Der LibreRID-Sender strahlt **parallel BLE4 Legacy** ab → auch ein
  Klassik-ESP32 kann die Demo-Drohne sehen. Reichweite dann geringer, aber sichtbar.
- Für reale DJI (die senden BLE5 LR) und für die Produktperspektive führt an
  **C3/S3/C6** kein Weg vorbei → für 10 € nachbestellen ist erlaubt und sinnvoll.
- **Offene Frage an Hermann: Welcher ESP32 liegt im Regal?** Danach entscheidet sich
  der Pfad für Woche 1.

### 2.2 Rest-Hardware v0

- Empfänger steht **fest** am Haus → eigene Position wird manuell im Dashboard gesetzt
  (GNSS-Modul erst v1; dann ATGM336H/NEO-M9-Klasse, 3-VA-Typ genügt).
- Strom: USB-Netzteil; Antenne: erst Board-PCB, später ggf. U.FL auf externe Dipol
  (das ist dann exakt das Gesprächsthema mit IK Elektronik).
- Sender-Seite: **LibreRID auf Chupito, fertig.** Kein zweiter Sender nötig.

## 3. Firmware-Architektur (Empfänger)

```
┌─ ESP32 ──────────────────────────────────────────────┐
│  BLE-Scanner (NimBLE, Passiv-Adv., Legacy + LR-Coded) │
│  WiFi-Monitor (NaN-/Beacon-Frames, Vendor-IEs:        │
│      DJI OUI 26:37:12 · ASTM FA:0B:BC · Franz. DRI)   │
│        │        ← Funk-Freiraum-Multiplex:           │
│        │          BLE und WiFi teilen EIN 2,4-GHz-Radio│
│        ▼                                              │
│  opendroneid-core-c (offizielle Referenz-Bibliothek,  │
│  MIT; dekodiert ASTM F3411 aus beiden Quellen)        │
│        ▼                                              │
│  JSON-Meldung {basis_id, ua_pos, pilot_pos, alt,      │
│  kurs, geschw, rssi, ts, empfaenger_id} ──► WLAN/Wired│
└───────────────────────────────────────────────────────┘
```

- Zeitmultiplex BLE↔WiFi nach dem Muster `DaringDroneDetector` (BLE-Scan pausiert WiFi):
  referenzimplementiert, nachbaubar.
- Transport nach draußen: **MQTT** (Mosquitto läuft bereits auf dem HomeLab/AGV-Watch) –
 Topic `rid/<empfaenger>`; Fallback HTTP-POST.
- **Referenzfunde (verifiziert 2026-11-08):**
  - `opendroneid/opendroneid-core-c` – Dekodier-Bibliothek; README verlinkt ESP32-
    Empfangsbeispiel, Linux-Scanner, Android-Receiver (`receiver-android`).
  - `ArduPilot/ArduRemoteID`, `peinser/esp-remoteid` – S3/C3-Sender-Firmware (EU-konform
    gebaut) als Referenz für das Sendeformat.
  - `sxjack/uav_electronic_ids` – ältere Sende-Bibliothek.
  - `danieltone/Drone-Detector-V2`, `Daring-Designs/DaringDroneDetector` – Fingerprint-
    und Multiplex-Techniken (Stufe 3-4, später).

## 4. Server-Seite (HomeLab) – die Liegenschaftswache

- **Ingest:** Mosquitto → kleiner Python-/Node-Konsument.
- **Speicher:** SQLite – Tabellen `sichtungen` (Rohevents) und `ereignisse` (Überflüge).
- **Dashboard (Leaflet + OSM-Kacheln):**
  - Live-Marker Drohne (Farbverlauf Alter des Signals) + Piloten-Marker,
  - Flugbahn-Spur der letzten N Minuten,
  - Polygon-Zeichenwerkzeug (GeoJSON), mehrere Polygone = späteres Lizenzmodell,
  - **Punkt-in-Polygon mit Puffermarge + Hysterie** (Eintritt erst nach 2 aufeinander-
    folgenden Punkten innerhalb Polygon+Rand, damit GPS-Rauschen keine Events flutet),
  - Event-Log mit Export (CSV + druckbares Beweis-Blatt PDF: Kennung, Zeit, Höhe,
    Dauer, Karte, **ausdrücklich mit Positionsunsicherheit**).
- Privacy-Default: alles im LAN, keine Cloud. (Das verkaufen wir später als Feature.)

## 5. Demo-Plan (Ziel: 20-Sekunden-Video)

1. Hardware klären (Regal-ESP32 oder C3-Bestellung) – **heute**
2. Empfänger bespielen, `receiver-android` aufs Handy als Referenz-Geiger (schnellster
   Sichtbeweis, dass LibreRID ankommt)
3. Node-Empfang → MQTT → Karte im Browser
4. Grundstück einzeichnen, Chupito-Runde mit LibreRID, Überflug-Event im Log
5. Video + Ton aus: „Eigenbau-Empfänger sieht eigene Drohne, zeichnet Überflug auf"
   → Unterlagen für IK-Gespräch und erste Errichter-Ansprache

## 6. Rechtlicher Rahmen für den Testbetrieb

- **Empfangen ist legal** (passives Mitlesen offener Kennungsaussendungen).
- Sende-Teil: LibreRID nutzt TEST-Seriennummer/Profil (README warnt selbst zutreffend:
  kein regulierungskonformes Add-on). Daraus folgt: Testflüge **nur auf dem eigenen
  Grundstück/Gelände**, Test-Kennung Clearly-TEST, kein Versuch, als konformer Add-on
  durchzugehen. Fürs Demo-Video unproblematisch, für später notieren: zertifizierter
  Sender für öffentliche Demos mit echten Drohnen.

## 7. Entscheidungspunkte

- **E1 (jetzt):** Welcher ESP32 liegt vor? → Pfad Woche 1 (Legacy-Empfang vs. C3-Kauf).
- **E2 (entschieden 2026-11-08):** RX wohnt als Unterprojekt `rx/` im LibreRID-Repo
  neben `tx/` – geteiltes Protokoll, geteilte Geschichte; ein späteres Ausgliedern bleibt möglich.
- **E3:** Dashboard-Sprache Python/Flask vs. Node/Express – Geschmack, beides trivial;
  Empfehlung Python (Mosquitto-Paho, Shapely für Polygon, ihr Standard-Ökosystem).
