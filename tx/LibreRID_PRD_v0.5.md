# LibreRID – Product Requirements Document (PRD)

**Status:** Draft 0.5  
**Projekt:** LibreRID  
**Positionierung:** LibreRID – Open Electronic Identification Module for RC Aircraft  
**Primäre Flight-Controller-Plattform (MVP):** INAV  
**Nachgelagerte Plattform:** Betaflight (nach dem ersten MVP)  
**Primärer Einsatz:** Nachrüstung bestehender FPV-Wings und Multicopter mit vorhandenem Flight Controller und GNSS  
**Repository:** `HermannHohenberger/libreRID`  
**Änderungen gegenüber 0.4:** siehe Anhang A

## 1. Produktvision

LibreRID ist eine kleine, kostengünstige und quelloffene Referenzplattform für elektronische Identifikation und Direct Remote ID bei bestehenden Eigenbau-Wings und FPV-Drohnen.

Das Modul wird an einen **dedizierten** freien UART des Flight Controllers angeschlossen, liest die notwendigen Flugdaten über MSP und erzeugt daraus OpenDroneID-Nachrichten.

**Sendepfade – klare Stufung:**

| Stufe | Sendepfad | Zweck |
|---|---|---|
| PoC / MVP | Bluetooth 4 Legacy Advertising | Interoperabilitätsnachweis und Funktionsdemo |
| Post-MVP (M3) | Bluetooth 5 Long Range/Extended Advertising, Wi-Fi Beacon, Wi-Fi NAN | Evaluation der für EU-DRI typischerweise relevanten Transportverfahren |

Bluetooth 4 Legacy Advertising allein gilt **nicht** als ausreichender EU-Direct-Remote-ID-Transportpfad. Das MVP weist Interoperabilität und Datenfluss nach; die Annäherung an EN‑4709‑002-taugliche Funkverfahren erfolgt erst nach dem MVP.

Der ideale Nutzerablauf:

1. Modul mit 5 V, GND, TX und RX verbinden.
2. Auf dem Flight Controller einen **dedizierten** freien UART für MSP konfigurieren.
3. Betreiber-ID und Modulseriennummer einmalig über USB-Konfiguration speichern.
4. Einschalten und fliegen.

Home Assistant, Cloud-Dienste, Benutzerkonten und Flugtracking gehören ausdrücklich nicht zum MVP.

## 2. Problem

Viele vorhandene FPV-Wings und Eigenbau-Drohnen besitzen bereits:

- einen Flight Controller mit INAV oder Betaflight,
- ein funktionierendes GNSS,
- einen freien UART,
- eine stabile 5-V-Stromversorgung.

Kommerzielle Remote-ID-Module bringen häufig nochmals GNSS, Akku, App und Cloud-Funktionen mit. Dadurch sind sie für eine einfache Nachrüstung vergleichsweise groß und teuer.

INAV und Betaflight stellen einen großen Teil der benötigten Flugdaten über MSP bereit, erzeugen aber selbst keinen vollständigen OpenDroneID-Broadcast für ein Direct-Remote-ID-Add-on. LibreRID nutzt MSP bewusst als **Eingangsprotokoll**; die OpenDroneID-Erzeugung und der Broadcast liegen im Modul, nicht im Flight Controller.

Der unmittelbare regulatorische Bedarf ist enger als die technische Zielgruppe: Privat gebaute Modelle benötigen in der offenen Kategorie der EU nicht generell Direct Remote ID. Relevante Einsatzfelder sind insbesondere technisch oder betrieblich notwendige Nachrüstungen, der Betrieb in der „specific category“, nationale Anforderungen, Forschung sowie eine offene Referenzimplementierung für Hersteller und Entwickler. LibreRID wird daher nicht mit einer allgemeinen Remote-ID-Pflicht für alle europäischen Modellflieger begründet.

## 3. Ziele

### 3.1 Proof-of-Concept-Ziele

- ESP32-S3 als feste Entwicklungsplattform
- Erzeugung statischer OpenDroneID-Testnachrichten
- Bluetooth 4 Legacy Advertising als erster und einziger Sendepfad
- reproduzierbarer Empfang und korrekte Dekodierung durch eine OpenDroneID-Testanwendung oder ein zweites Testgerät
- dokumentierter Build-, Flash- und Testablauf

### 3.2 MVP-Ziele

- Fokus auf **INAV** über MSP und OpenDroneID-Nachrichten nach dem OpenDroneID-Datenmodell
- Datenfelder und Semantik so wählen, dass sie später auf ein EU-DRI-Profil (Delegated Regulation (EU) 2019/945, EN 4709-002) abbildbar sind – **ohne** dass das MVP selbst Konformität beansprucht
- Anschluss an einen **dedizierten** freien UART des Flight Controllers
- Versorgung mit 5 V
- Übernahme von Position, Höhen (siehe 6.3.1), Kurs, Geschwindigkeit, Satellitenstatus und Arm-Status
- Erfassung des Startpunkts beim Übergang von disarmed zu armed, sofern ein gültiger GNSS-Fix vorliegt
- Speicherung der UAS-Betreibernummer
- Prüfung der Betreiber-ID auf plausibles Format und Prüfsumme, soweit öffentlich spezifiziert
- eindeutige Modulseriennummer; Zielformat ANSI/CTA-2063, solange die Vergabe geklärt ist (sonst dokumentierter Entwicklungsplatzhalter)
- syntaktisch und semantisch plausible OpenDroneID-Nachrichten
- Statusanzeige über eine mehrfarbige LED
- Konfiguration ohne Smartphone-App
- Firmware-Update über USB
- Persistenz von Betreiber-ID und Seriennummer vor dem ersten echten Flugtest

### 3.3 Spätere Ziele

- Bluetooth 5 Long Range/Extended Advertising sowie Wi-Fi Beacon oder Wi-Fi NAN, soweit für das angestrebte regulatorische Profil erforderlich und auf der Zielplattform zuverlässig realisierbar
- Betaflight-Unterstützung über MSP
- automatische Erkennung des Flight-Controller-Typs
- zusätzliche Betriebsarten für das französische nationale elektronische Identifikationssystem und weitere internationale Standards, zum Beispiel FAA Remote ID
- länder- beziehungsweise standardspezifische Encoder und Profile bei möglichst gemeinsamer Hardware
- Anforderungsmatrix für EU, Frankreich und USA als eigenes Arbeitspaket
- MAVLink-Eingang für ArduPilot und PX4
- CRSF-Telemetrie als alternative Datenquelle
- optionaler eigener GNSS-Empfänger als Fallback
- OTA-Update
- Konfigurationsseite über Bluetooth oder WLAN
- eigenes kleines PCB
- dokumentierter Konformitätsprozess und Referenzhardware
- optionales Arming-Feedback an den Flight Controller (nur falls FC und Protokoll das unterstützen)

## 4. Nicht-Ziele des MVP

- Home Assistant
- Cloud-Anbindung
- Benutzerkonto
- Mobilfunk
- Netzwerk-Remote-ID
- Flugbuch oder Trackingportal
- eigene iOS-/Android-App
- Unterstützung von Flight Controllern ohne GNSS
- **geteilter UART** mit OSD, GPS oder anderen Peripheriegeräten (MVP verlangt einen dedizierten UART)
- **Arming-Inhibit / RID-ready-Rückmeldung** an den Flight Controller (LibreRID ist im MVP reiner MSP-Consumer; fehlende RID-Bereitschaft wird nur per LED und ggf. ODID-Status signalisiert)
- Behauptung, Bluetooth 4 Legacy Advertising erfülle allein die EU-DRI-Transportanforderungen
- Behauptung oder Vermarktung als rechtskonformes Direct-Remote-ID-Add-on vor abgeschlossener Konformitätsbewertung
- Verkauf als bereits CE-konformes Endprodukt ohne abgeschlossene Konformitätsbewertung

## 5. Zielnutzer

### MVP (primär)

- Besitzer bestehender FPV-Wings mit **INAV** und GNSS
- Open-Source-Entwickler und Hersteller, die eine offene Referenzimplementierung benötigen

### Nach dem MVP (sekundär)

- Besitzer von FPV-Multicoptern mit **Betaflight** und GNSS
- Modellbauer, die ein kleines Nachrüstmodul für weitere FC-Plattformen suchen

## 6. Kernanforderungen

### 6.1 Elektrische Schnittstelle

Mindestanschluss:

| LibreRID | Flight Controller |
|---|---|
| 5V | 5-V-Ausgang |
| GND | GND |
| RX | TX des FC-UART |
| TX | RX des FC-UART |

Anforderungen:

- Eingangsspannung: 4,5 bis 5,5 V
- Logikpegel UART: 3,3 V (5-V-UART-FCs sind Out-of-Scope des MVP)
- Verpolungs- und Überspannungsschutz für die Serienhardware
- JST-SH 1.0 oder JST-GH als bevorzugter Stecker
- Lötpads als zusätzliche Anschlussmöglichkeit
- Stromaufnahme (Richtwerte, zu messen und zu dokumentieren):
  - Idle / BLE4-Advertising: Ziel möglichst unter 100 mA Mittelwert
  - Peak (Wi-Fi/BLE-Sendespitzen, spätere Profile): Auslegung der Versorgung und des Puffers auf Spitzen bis ca. 500 mA
- Masse Prototyp: möglichst unter 10 g ohne Gehäuse und Kabel
- Masse Revision A: unter 6 g ohne Kabel

### 6.2 Flight-Controller-Protokoll

MVP:

- MSP v2 bevorzugt
- MSP v1 als Fallback
- UART-Baudraten: 115200 Standard, optional 57600
- **dedizierter UART**; kein Sharing mit OSD/GPS im MVP
- Polling oder Subscriptions so, dass der FC nicht unnötig belastet wird
- konkrete MSP-Nachrichten und Raten werden in M0 festgelegt und dokumentiert

Benötigte Daten:

- GNSS-Fix
- Satellitenzahl
- Breitengrad und Längengrad
- GNSS-Höhe (ellipsoid / wie vom FC geliefert; Mapping siehe 6.3.1)
- Höhe relativ zum Startpunkt (Takeoff / Home, wie vom FC bzw. LibreRID berechnet)
- Geschwindigkeit über Grund
- Kurs über Grund
- Arm-Status
- Zeitstempel, sofern verfügbar

### 6.3 Remote-ID-Ausgabe

Die Firmware soll die OpenDroneID-Core-C-Bibliothek verwenden oder sich eng daran orientieren.

**PoC / MVP-Ausgabe:**

- Bluetooth 4 Legacy Advertising (Nachrichtenzyklus gemäß OpenDroneID-Empfehlungen für Legacy Advertising, typisch mehrere Einzelmessages pro Sekunde)

**Nach dem MVP getrennt zu evaluieren (M3):**

- Bluetooth 5 Extended/Long Range
- Wi-Fi Beacon
- Wi-Fi NAN

Die Auswahl der später gleichzeitig unterstützten Sendepfade richtet sich nach dem angestrebten regulatorischen Profil (insbesondere EN 4709-002), der Plattformunterstützung, der Empfangskompatibilität und den Ergebnissen von Reichweiten- und Interoperabilitätstests.

Zu übertragende Daten:

- UAS-Betreibernummer
- eindeutige Seriennummer des LibreRID-Moduls (Ziel: ANSI/CTA-2063)
- Zeitstempel
- Position des Luftfahrzeugs
- Höhen gemäß Höhenmapping (6.3.1)
- Kurs
- Geschwindigkeit über Grund
- Position des Fernpiloten **ersatzweise als Startpunkt** (MVP-Default; siehe 6.3.2)
- Status beziehungsweise Notfallstatus / Error-Flags bei Datenverlust

#### 6.3.1 Höhenmapping (MVP)

„Höhe über Oberfläche“ ist kein eigener Sensorpfad im MVP. Stattdessen gilt:

| OpenDroneID-Konzept (sinngemäß) | MVP-Quelle |
|---|---|
| Geodätische / GNSS-Höhe | GNSS-Höhe aus MSP, so wie der FC sie liefert |
| Höhe über Startpunkt (Takeoff) | Differenz zur beim Armen gespeicherten Startpunkt-Höhe, oder FC-relative Höhe falls konsistent verfügbar |
| Höhe über Gelände / AGL | **nicht** im MVP; kein Terrain-Modell |

Die gewählte Abbildung wird in der Firmware und der Testdokumentation festgehalten, damit Empfangsanzeigen reproduzierbar mit INAV verglichen werden können.

#### 6.3.2 Piloten- / Operator-Position (MVP)

MVP-Default: Beim Armen mit gültigem GNSS-Fix wird die aktuelle Position als **Startpunkt** gespeichert und als Ersatz für die Fernpilotenposition verwendet, sofern keine externe Quelle konfiguriert ist.

Eine Live-Pilotenposition (z. B. über Bodenstation) ist kein MVP-Ziel und bleibt eine offene Entscheidung für spätere Profile.

### 6.4 Verhalten beim Start

1. Modul bootet.
2. Konfiguration, Betreiber-ID und Seriennummer werden geprüft.
3. Verbindung zum Flight Controller wird aufgebaut.
4. Auf gültigen GNSS-Fix wird gewartet.
5. Der Arm-Status des Flight Controllers wird überwacht.
6. Beim Übergang von disarmed zu armed wird bei gültigem GNSS-Fix der Startpunkt unveränderlich für den laufenden Betriebszyklus gespeichert.
7. Ohne gültigen Fix beim Armen wird nicht stillschweigend mit einer alten oder beliebigen Position gesendet; stattdessen wird ein definierter Fehlerzustand signalisiert (LED und ODID-Status).
8. Der Broadcast beginnt **vor dem Abheben** (sobald Daten und Konfiguration gültig sind bzw. spätestens mit Armen bei gültigem Fix) und läuft während des Fluges kontinuierlich.
9. Bei Datenverlust wird ein definierter Fehlerstatus gesendet und über LED angezeigt.
10. Das MVP **verhindert das Armen des FC nicht**; der Pilot muss die LED-/Statusanzeige beachten.

### 6.5 Konfiguration

MVP-Konfiguration vorzugsweise über USB-Massenspeicher oder USB-Weboberfläche. Die Wahl ist eine offene Entscheidung; beide Wege müssen Betreiber-ID und Seriennummer vor dem ersten Flugtest persistieren können.

Beispiel `config.json`:

```json
{
  "operator_id": "TEST-OPERATOR-ID-NOT-FOR-FLIGHT",
  "serial_number": "DEV-PLACEHOLDER-SN",
  "aircraft_type": "fixed_wing",
  "input_protocol": "msp",
  "uart_baud": 115200,
  "operator_location_mode": "takeoff_point",
  "broadcast": {
    "bluetooth_legacy": true,
    "bluetooth_5": false,
    "wifi_beacon": false,
    "wifi_nan": false
  }
}
```

Sicherheitsanforderungen:

- Betreiber-ID nicht im öffentlichen Repository hinterlegen
- Konfiguration nach dem Speichern gegen unbeabsichtigte Änderung schützen
- ungültige oder fehlende Betreiber-ID klar signalisieren
- Möglichkeit zum Zurücksetzen nur durch physischen Tastendruck oder USB-Verbindung
- Entwicklungsplatzhalter-Seriennummern und Test-Betreiber-IDs müssen in Empfangs- und Flugtests als solche erkennbar sein

### 6.6 Statusanzeige

Vorschlag:

- Blau blinkend: Start
- Gelb blinkend: Flight Controller nicht erkannt
- Gelb dauerhaft: GNSS noch ohne Fix
- Grün blinkend: gültige Daten, noch nicht armed
- Grün dauerhaft: Remote ID aktiv und vollständig
- Rot blinkend: Konfiguration ungültig (fehlende/ungültige Betreiber-ID oder Seriennummer)
- Rot dauerhaft: schwerer Fehler oder Datenverlust im Flug

## 7. Empfohlene Prototyp-Hardware

### 7.1 Controller

**Festgelegt für Proof of Concept und MVP: ESP32-S3**

Gründe:

- Bluetooth 5 inklusive Extended beziehungsweise Long Range (für Post-MVP-Evaluation)
- Wi-Fi
- ausreichende Rechenleistung
- UART und USB
- bestehende OpenDroneID-Referenzsoftware
- geringe Kosten und gute Verfügbarkeit

Für den ersten Breadboard-Prototyp:

- ESP32-S3-DevKitC-1 beziehungsweise das konkret bestellte ESP32-S3-Entwicklungsboard
- genaue Modul-, Flash- und PSRAM-Variante vor Beginn dokumentieren

Der klassische ESP32 ist für das Projekt nicht erste Wahl, weil Bluetooth 5 Long Range und Extended Advertising fehlen – Fähigkeiten, die LibreRID für die spätere EU-Transportpfad-Evaluation benötigt.

### 7.2 Weitere Bauteile

Für den ersten Prototyp:

- ESP32-S3-Devboard
- 4-poliges UART-Kabel
- 470-µF-Pufferkondensator an 5 V (Brownout-Schutz an geteilter FC-5-V-Schiene neben VTX/ELRS)
- 100-nF-Abblockkondensator
- optionale Schottky-Diode oder Sicherung
- eine RGB-LED, falls nicht auf dem Board vorhanden
- Taster für Reset/Config
- USB-Kabel

Kein eigener GNSS-Empfänger im MVP.

### 7.3 Zielhardware für Revision A

- bevorzugt ESP32-S3-MINI-1; ESP32-C3-MINI-1 bleibt eine spätere Optimierungsoption
- 5-V-zu-3,3-V-Regler mit ausreichender Reserve für Wi-Fi-Sendespitzen
- ESD-Schutz an USB und UART
- JST-SH-1.0-4P oder JST-GH-4P
- USB-C
- RGB-LED
- Config/Boot-Taster
- eindeutige Seriennummer im nichtflüchtigen Speicher
- kleine PCB-Antenne oder zertifiziertes Funkmodul mit integrierter Antenne

Zielgröße: etwa 25 × 18 mm  
Zielmasse: unter 6 g ohne Kabel  
Ziel-BOM bei Kleinserie: unter 20 Euro

## 8. Softwarearchitektur

```text
┌───────────────────────────────┐
│ Flight Controller             │
│ MVP: INAV + GNSS              │
└───────────────┬───────────────┘
                │ MSP über dedizierten UART
┌───────────────▼───────────────┐
│ Input Adapter                 │
│ MSP v1 / MSP v2               │
└───────────────┬───────────────┘
                │ normalisierte Flugdaten
┌───────────────▼───────────────┐
│ Flight State Manager          │
│ Fix, Takeoff/Home, Armed,     │
│ Höhenmapping, Fehler          │
└───────────────┬───────────────┘
                │
┌───────────────▼───────────────┐
│ Identification Encoder        │
│ MVP: OpenDroneID              │
│ Später: EU-Profilschärfung,   │
│ FR- und weitere Profile       │
└───────────────┬───────────────┘
                │
┌───────────────▼───────────────┐
│ Broadcast Adapter             │
│ PoC/MVP: BLE4 Legacy          │
│ Post-MVP: BLE5, Wi-Fi         │
└───────────────────────────────┘
```

## 9. Abnahmekriterien MVP

Das MVP gilt als **technisch** erfolgreich, wenn:

1. Ein SpeedyBee F405 Wing Mini mit INAV über einen **dedizierten** UART angeschlossen werden kann.
2. LibreRID erkennt MSP und erhält innerhalb von fünf Sekunden gültige FC-Daten.
3. Betreiber-ID und Modulseriennummer sind vor dem Flugtest konfiguriert und überleben einen Neustart.
4. Die ausgesendeten OpenDroneID-Daten werden von einer geeigneten Testanwendung oder einem zweiten Testgerät reproduzierbar empfangen und dekodiert (BLE4).
5. Position, Höhen gemäß 6.3.1, Kurs, Geschwindigkeit, Startpunkt und Seriennummer werden korrekt angezeigt.
6. Der Broadcast beginnt vor dem Abheben und läuft nach dem Armen durchgehend mit den für BLE4 Legacy Advertising konfigurierten Aktualisierungsintervallen.
7. Ein Ausfall der UART-Daten wird innerhalb einer definierten Zeit erkannt und signalisiert (LED und ODID-Status).
8. Ein Neustart verliert weder Betreiber-ID noch Modulseriennummer.

**Ausdrücklich nicht Teil der MVP-Abnahme:**

- Erfüllung der EU-DRI-Transportanforderungen allein durch BLE4
- CE-Konformität oder Konformitätsvermutung nach EN 4709-002
- Arming-Inhibit durch LibreRID
- Betaflight-Unterstützung

Der Empfang in einer App und das Bestehen dieser technischen Kriterien weisen Interoperabilität nach, jedoch keine regulatorische Konformität.

## 10. Testplan

### Labortests

- MSP-Testdaten aus aufgezeichneten INAV-Sessions
- ungültige GNSS-Daten
- wechselnder Fix-Status
- UART-Unterbrechung
- Stromunterbrechung / Brownout-Simulation an 5 V
- falsche oder fehlende Betreiber-ID
- fehlende oder Platzhalter-Seriennummer
- BLE-4-Dauerbroadcast und Nachrichtenzyklus
- Dauerlauf über mindestens zwei Stunden
- Vergleich GNSS-Höhe und Takeoff-relative Höhe mit INAV-Referenzwerten

### Bodentest

- Empfang mit mindestens zwei unterschiedlichen Android-Geräten
- Vergleich der angezeigten Position und Höhen mit INAV
- Reichweitentest am Boden
- Störtest neben VTX, ELRS und GPS

### Flugtest (MVP)

- Wing mit INAV
- Startpunkt-Erfassung beim Armen
- Positionsverlauf
- Datenverlust
- Reichweite
- Verhalten bei Disarm und Landung

### Erweiterungstest (nach MVP)

- Copter mit Betaflight
- BLE5 / Wi-Fi Beacon / Wi-Fi NAN gemäß M3

## 11. Recht und Konformität

LibreRID soll langfristig technisch auf die Anforderungen für Zusatzgeräte zur direkten Fernidentifizierung ausgerichtet werden. Maßgebliche Bezugspunkte sind insbesondere:

- Delegated Regulation (EU) 2019/945 (Anforderungen an UAS und Direct-Remote-ID-Add-ons)
- EN 4709-002 (Aerospace – UAS – Direct Remote Identification), inklusive bekannter Einschränkungen zur Konformitätsvermutung gegenüber „empfangbar durch bestehende Mobilgeräte“
- OpenDroneID als technische Implementierungsbasis

**Wichtige Unterscheidung für dieses Projekt:**

| Ebene | Bedeutung |
|---|---|
| OpenDroneID-Nachrichten über BLE4 | Interoperabilitäts- und Funktionsnachweis (PoC/MVP) |
| EU-DRI-taugliche Transportverfahren (typisch BLE5 LR, Wi-Fi Beacon, Wi-Fi NAN) | Post-MVP-Evaluation (M3) |
| CE-gekennzeichnetes Add-on | erst nach Konformitätsbewertung (M6) |

Eine Open-Source-Firmware oder ein funktionsfähiger Prototyp ist nicht automatisch ein rechtskonformes, CE-gekennzeichnetes Produkt.

Für ein als fertiges Zusatzgerät in Verkehr gebrachtes Produkt sind unter anderem erforderlich:

- technische Dokumentation
- Konformitätsbewertung
- EU-Konformitätserklärung
- CE-Kennzeichnung
- eindeutige physische Seriennummer (ANSI/CTA-2063 und Herstellervergabe)
- Herstellerangaben und Anleitung
- Prüfung der Betreiber-ID
- Maßnahmen gegen Manipulation
- Einhaltung der Funkanlagen- und EMV-Anforderungen

Das Projekt unterscheidet deshalb zwei Ebenen:

1. **Open-Source-Referenzimplementierung und Eigenbauprototyp**
2. **Geprüfte LibreRID-Referenzhardware beziehungsweise späteres Serienprodukt**

Open-Source-Firmware und Manipulationsschutz stehen in einem natürlichen Spannungsverhältnis: Eine frei flashbare Referenz kann Schutzziele eines Serienprodukts nicht vollständig erfüllen. Manipulationserschwerung ist daher primär ein Thema der Produkt-/Konformitätsebene (M6), nicht des MVP.

Bis zur abgeschlossenen Konformitätsbewertung darf das Projekt nicht behaupten, ein Prototyp sei bereits EU-konform. Auch der erfolgreiche Empfang in einer OpenDroneID-App ist nur ein Interoperabilitätsnachweis und kein Konformitätsnachweis.

## 12. Offene Entscheidungen

- genaue ESP32-S3-Board- und Modulvariante für den Prototyp
- ESP32-C3 als mögliche spätere Zielplattform für optimierte Serienhardware
- ESP-IDF oder Arduino/PlatformIO
- Auswahl zusätzlicher Broadcast-Verfahren nach Einzelbewertung von Bluetooth 5, Wi-Fi Beacon und Wi-Fi NAN
- USB-Massenspeicher versus USB-Webkonfiguration
- genaue MSP-Nachrichten und Aktualisierungsraten
- Live-Pilotenposition als optionale spätere Quelle (MVP-Default: Startpunkt)
- Strategie zur Manipulationserschwerung auf Produktebene
- Erzeugung und Verwaltung der ANSI/CTA-2063-konformen Seriennummern (Manufacturer Code / Vergabe)
- Lizenzmodell für Firmware, Hardware und Dokumentation
- ob und wie ein spätes Arming-Feedback an unterstützte Flight Controller angebunden wird

## 13. Vorgeschlagene Meilensteine

### M0 – Grundlagen

- Rechts- und Standardanforderungen dokumentieren (2019/945, EN 4709-002, OpenDroneID)
- MSP-Daten für INAV bestimmen; Betaflight-Anforderungen für M4 vormerken
- Höhenmapping und Startpunkt-Semantik festschreiben
- OpenDroneID-Core-C evaluieren

### M1 – Statischer Sender-Demo

- ESP32-S3 sendet statische OpenDroneID-Testdaten über Bluetooth 4 Legacy Advertising
- reproduzierbarer Empfang und Dekodierung
- Build-, Flash- und Testablauf dokumentieren

### M2 – Identität, Konfiguration und Status

- Betreiber-ID (Validierung Format/Prüfsumme soweit spezifiziert)
- Modulseriennummer (CTA-2063-Ziel oder dokumentierter Platzhalter)
- USB-Konfiguration und Persistenz
- Status-LED
- Voraussetzung für alle nachfolgenden Flugtests

### M3 – EU-relevante Broadcast-Verfahren (Evaluation)

- Bluetooth 5 Long Range/Extended Advertising evaluieren
- Wi-Fi Beacon und Wi-Fi NAN getrennt evaluieren
- Empfangskompatibilität, Reichweite und Plattformzuverlässigkeit dokumentieren
- Sendepfade für das angestrebte regulatorische Profil auswählen
- Parallel zu M2/M4 möglich; nicht blockierend für den ersten INAV-Flugtest mit BLE4

### M4 – INAV-Anbindung (MVP-Kern)

- MSP über dedizierten UART
- echte GNSS- und Flugdaten
- SpeedyBee F405 Wing Mini als Referenz
- Startpunkt beim Armen mit gültigem GNSS-Fix erfassen
- Höhenmapping gemäß 6.3.1
- MVP-Abnahmekriterien (Kap. 9) erfüllen

### M5 – Betaflight-Anbindung

- MSP-Unterschiede zu INAV
- optional automatisches Protokoll- und FC-Erkennen
- Copter-Flugtest

### M6 – Referenz-PCB

- Schaltplan
- PCB
- Gehäuse
- BOM und Bauanleitung

### M7 – Konformitäts-Roadmap

- Gap-Analyse (Protokoll, Transport, Seriennummer, Manipulationsschutz, Funk/EMV)
- notwendige Funk-, EMV- und Produkttests
- technische Dokumentation
- Entscheidung über ein marktfähiges Modul

### M8 – Internationale Anforderungsmatrix (künftiges Arbeitspaket)

- verbindliche technische und regulatorische Anforderungen für EU, Frankreich und USA ermitteln
- geforderte Identifikatoren, Nachrichtentypen, Datenfelder und Aktualisierungsraten vergleichen
- Funkverfahren, Reichweiten-, Seriennummern-, Manipulationsschutz- und Konformitätsanforderungen gegenüberstellen
- gemeinsame Hardwarefähigkeiten und länderspezifische Abweichungen dokumentieren
- Anforderungen getrennten Encodern beziehungsweise Profilen zuordnen
- Auswirkungen auf Firmwarearchitektur, Testplan und Konformitätsnachweise ableiten

Geplante Struktur der Matrix:

| Anforderungsbereich | EU | Frankreich | USA |
|---|---|---|---|
| System/Standard | EU Direct Remote ID / OpenDroneID | nationales elektronisches Identifikationssystem | FAA Remote ID |
| Identifikatoren und Datenfelder | zu ermitteln | zu ermitteln | zu ermitteln |
| Nachrichtenformat und Aktualisierungsraten | zu ermitteln | zu ermitteln | zu ermitteln |
| Funkverfahren und Reichweite | zu ermitteln | zu ermitteln | zu ermitteln |
| Hardwareanforderungen | gemeinsame Basis und Abweichungen ermitteln | gemeinsame Basis und Abweichungen ermitteln | gemeinsame Basis und Abweichungen ermitteln |
| Konformität und Nachweise | zu ermitteln | zu ermitteln | zu ermitteln |

Die Matrix ist eine geplante Untersuchung und keine Aussage über bereits implementierte oder abschließend bewertete Konformität.

**Meilenstein-Reihenfolge (Kurz):**  
M0 → M1 → M2 (Identität) → M4 (INAV/MVP) ; M3 (Transport-Evaluation) parallel ; danach M5–M8.

## 14. Projektprinzipien

LibreRID soll nicht als „günstiger DroneTag-Klon“ positioniert werden.

Stattdessen verfolgt das Projekt folgende Leitgedanken:

- offene Referenzimplementierung für Direct Remote ID
- einfache Nachrüstung bestehender Modelle
- möglichst keine Änderungen am Flight Controller
- Nutzung vorhandener Sensoren (insbesondere GNSS)
- Plug-and-Play über einen dedizierten freien UART
- offene Hardware und offene Firmware
- klare Trennung zwischen Referenzimplementierung und später möglichen kommerziellen Produkten
- klare Trennung zwischen Interoperabilitätsnachweis und Konformitätsnachweis

### Designprinzip

Der Idealfall für den Anwender:

1. Modul anschließen (5 V, GND, TX, RX)
2. Dedizierten UART im Flight Controller auf MSP konfigurieren
3. Betreiber-ID einmalig eintragen
4. Fliegen

Keine Smartphone-App ist für das MVP erforderlich.

## 15. Repository-Struktur

Um das Projekt langfristig wartbar zu halten, wird eine modulare Struktur empfohlen.

### LibreRID-Core

Plattformunabhängige Bibliothek:

- OpenDroneID-Datenmodell
- Konfiguration
- Statusverwaltung
- Höhen- und Startpunktlogik
- Nachrichtenaufbereitung
- Tests

### LibreRID-Firmware

Hardwareabhängige Firmware:

- ESP32-S3; ESP32-C3 später optional
- MSP
- MAVLink (später)
- Bluetooth
- Wi-Fi
- USB-Konfiguration

### LibreRID-Hardware

Hardwareentwicklung:

- KiCad
- Schaltpläne
- PCB
- BOM
- Gehäuse
- Fertigungsunterlagen

## 16. Hardwarestrategie

Für den MVP wird ausdrücklich **kein eigenes GPS** vorgesehen.

Begründung:

- nahezu alle geeigneten Wings besitzen bereits ein GNSS
- geringeres Gewicht
- geringere Kosten
- geringerer Stromverbrauch
- einfachere Montage

Risiken ohne eigenes GNSS (akzeptiert im MVP, zu beobachten in Tests):

- Abhängigkeit von FC-Fix-Qualität
- Störungen durch VTX/ELRS in der Nähe der FC-Antenne
- Abweichungen zwischen FC-Home und LibreRID-Startpunkt-Logik

Ein optionales GNSS-Modul kann später als Fallback ergänzt werden.

### Referenzplattform

Referenzplattform für alle frühen Entwicklungen:

- ESP32-S3-DevKit beziehungsweise konkret bestelltes ESP32-S3-Entwicklungsboard
- UART
- USB-C
- Bluetooth 5
- Wi-Fi

Der ESP32-C3 bleibt eine mögliche spätere Zielplattform, ist aber nicht Bestandteil des ersten Proof of Concept oder MVP.

Erst nach erfolgreichem Feldtest erfolgt die Entwicklung einer eigenen kompakten Leiterplatte.

## 17. Langfristige Vision

LibreRID bleibt der Projektname und wird als **„LibreRID – Open Electronic Identification Module for RC Aircraft“** positioniert.

LibreRID soll sich langfristig zu einer offenen Referenzplattform für die elektronische Identifikation von RC-Luftfahrzeugen entwickeln. Der Einsatzbereich soll nicht auf EU Direct Remote ID beschränkt bleiben. Als spätere Betriebsarten werden berücksichtigt:

- EU Direct Remote ID über OpenDroneID
- das französische nationale elektronische Identifikationssystem
- weitere internationale Standards, zum Beispiel FAA Remote ID in den USA

Die Hardware soll dabei über Regionen und Betriebsarten hinweg möglichst gleich bleiben. Länderspezifische Datenmodelle, Nachrichtenformate und regulatorische Anforderungen sollen durch getrennte Encoder beziehungsweise Profile umgesetzt werden. Eine normalisierte interne Darstellung der Flug- und Identifikationsdaten bildet dafür die gemeinsame Grundlage.

Der Proof of Concept weist zunächst ausschließlich die Erzeugung und den Empfang statischer OpenDroneID-Testdaten über Bluetooth 4 nach. Der erste MVP ergänzt INAV, MSP, Persistenz von Betreiber-ID/Seriennummer und einen durchgehenden BLE4-Broadcast mit echten Flugdaten. Betaflight, EU-relevante zusätzliche Broadcast-Verfahren und internationale Profile sind nachgelagerte Erweiterungen.

Zielplattformen:

- INAV (MVP)
- Betaflight (nach MVP)
- ArduPilot (später)
- PX4 (später)

Das Projekt soll so aufgebaut werden, dass andere Entwickler eigene Hardware oder Firmware-Erweiterungen entwickeln können, ohne den Kern verändern zu müssen.

---

## Anhang A – Änderungen in Draft 0.5

Gegenüber Draft 0.4:

1. **BLE4 vs. EU klar getrennt:** MVP = Interoperabilitätsdemo; EU-taugliche Transportverfahren erst Post-MVP (M3).
2. **Zielnutzer geschärft:** MVP primär INAV; Betaflight sekundär nach dem MVP.
3. **Dedizierter UART** statt „frei oder teilbar“; Shared-UART als MVP-Nicht-Ziel.
4. **Arming-Inhibit** ausdrücklich Out-of-Scope des MVP.
5. **Höhenmapping** und **Pilotenposition = Startpunkt** spezifiziert (6.3.1 / 6.3.2).
6. **Stromaufnahme** in Idle/Mittelwert und Peak getrennt.
7. **Meilensteine umgeordnet:** Identität/Konfiguration (M2) vor INAV-Flugtests (M4); Broadcast-Evaluation parallel als M3.
8. **Normbezüge** 2019/945 und EN 4709-002 in Kap. 11 ergänzt.
9. **Abnahmekriterien** um Persistenz und explizite Nicht-Abnahmen ergänzt.
10. **Open-Source vs. Manipulationsschutz** als Spannungsverhältnis dokumentiert.
11. **CTA-2063-Seriennummer** als Ziel mit dokumentiertem Entwicklungsplatzhalter.
12. **config.json** um Seriennummer, `operator_location_mode` und `wifi_nan` ergänzt.
