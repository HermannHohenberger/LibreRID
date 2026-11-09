# LibreRID – Product Requirements Document (PRD)

**Status:** Draft 0.4  
**Projekt:** LibreRID  
**Positionierung:** LibreRID – Open Electronic Identification Module for RC Aircraft  
**Primäre Flight-Controller-Plattform:** INAV; Betaflight folgt nach dem ersten MVP  
**Primärer Einsatz:** Nachrüstung bestehender FPV-Wings und Multicopter mit vorhandenem Flight Controller und GNSS  
**Repository:** `HermannHohenberger/libreRID`

## 1. Produktvision

LibreRID ist eine kleine, kostengünstige und quelloffene Referenzplattform für elektronische Identifikation und Direct Remote ID bei bestehenden Eigenbau-Wings und FPV-Drohnen.

Das Modul wird an einen freien UART des Flight Controllers angeschlossen, liest die notwendigen Flugdaten über MSP und erzeugt daraus OpenDroneID-Nachrichten. Der erste Prototyp sendet diese über Bluetooth 4 Legacy Advertising. Weitere zulässige Übertragungsverfahren werden erst nach dem grundlegenden Funktionsnachweis einzeln evaluiert.

Der ideale Nutzerablauf:

1. Modul mit 5 V, GND, TX und RX verbinden.
2. Auf dem Flight Controller einen freien UART für MSP konfigurieren.
3. Betreiber-ID einmalig über eine Konfigurationsdatei oder USB-Weboberfläche speichern.
4. Einschalten und fliegen.

Home Assistant, Cloud-Dienste, Benutzerkonten und Flugtracking gehören ausdrücklich nicht zum MVP.

## 2. Problem

Viele vorhandene FPV-Wings und Eigenbau-Drohnen besitzen bereits:

- einen Flight Controller mit INAV oder Betaflight,
- ein funktionierendes GNSS,
- einen freien oder teilbaren UART,
- eine stabile 5-V-Stromversorgung.

Kommerzielle Remote-ID-Module bringen häufig nochmals GNSS, Akku, App und Cloud-Funktionen mit. Dadurch sind sie für eine einfache Nachrüstung vergleichsweise groß und teuer.

INAV und Betaflight stellen einen großen Teil der benötigten Flugdaten über MSP bereit, erzeugen aber selbst keinen vollständigen OpenDroneID-Broadcast für ein Direct-Remote-ID-Add-on.

Der unmittelbare regulatorische Bedarf ist enger als die technische Zielgruppe: Privat gebaute Modelle benötigen in der offenen Kategorie der EU nicht generell Direct Remote ID. Relevante Einsatzfelder sind insbesondere technisch oder betrieblich notwendige Nachrüstungen, der Betrieb in der „specific category“, nationale Anforderungen, Forschung sowie eine offene Referenzimplementierung für Hersteller und Entwickler. LibreRID wird daher nicht mit einer allgemeinen Remote-ID-Pflicht für alle europäischen Modellflieger begründet.

## 3. Ziele

### 3.1 Proof-of-Concept-Ziele

- ESP32-S3 als feste Entwicklungsplattform
- Erzeugung statischer OpenDroneID-Testnachrichten
- Bluetooth 4 Legacy Advertising als erster und einziger Sendepfad
- reproduzierbarer Empfang und korrekte Dekodierung durch eine OpenDroneID-Testanwendung oder ein zweites Testgerät
- dokumentierter Build-, Flash- und Testablauf

### 3.2 MVP-Ziele

- Fokus auf INAV/MSP und OpenDroneID-Nachrichten nach den für den Prototyp ausgewählten EU-Protokollanforderungen
- Anschluss an einen freien UART des Flight Controllers
- Versorgung mit 5 V
- Unterstützung von INAV über MSP
- Übernahme von Position, Höhe, Kurs, Geschwindigkeit, Satellitenstatus und Arm-Status
- Erfassung des Startpunkts beim Übergang von disarmed zu armed, sofern ein gültiger GNSS-Fix vorliegt
- Speicherung der UAS-Betreibernummer
- Prüfung der Betreiber-ID auf plausibles Format und Prüfsumme, soweit öffentlich spezifiziert
- eindeutige physische Seriennummer pro Modul
- syntaktisch und semantisch plausible OpenDroneID-Nachrichten in der ausgewählten Protokollversion
- Statusanzeige über eine mehrfarbige LED
- Konfiguration ohne Smartphone-App
- Firmware-Update über USB

### 3.3 Spätere Ziele

- zusätzliche Betriebsarten für das französische nationale elektronische Identifikationssystem und weitere internationale Standards, zum Beispiel FAA Remote ID
- länder- beziehungsweise standardspezifische Encoder und Profile bei möglichst gemeinsamer Hardware
- Anforderungsmatrix für EU, Frankreich und USA als eigenes Arbeitspaket
- MAVLink-Eingang für ArduPilot und PX4
- CRSF-Telemetrie als alternative Datenquelle
- Betaflight-Unterstützung über MSP
- automatische Erkennung des Flight-Controller-Typs
- optionaler eigener GNSS-Empfänger als Fallback
- OTA-Update
- Konfigurationsseite über Bluetooth oder WLAN
- eigenes kleines PCB
- dokumentierter Konformitätsprozess und Referenzhardware
- Bluetooth 5 Long Range/Extended Advertising sowie Wi-Fi Beacon oder Wi-Fi NAN, soweit für den angestrebten Einsatz erforderlich und auf der Zielplattform zuverlässig realisierbar

## 4. Nicht-Ziele des MVP

- Home Assistant
- Cloud-Anbindung
- Benutzerkonto
- Mobilfunk
- Netzwerk-Remote-ID
- Flugbuch oder Trackingportal
- eigene iOS-/Android-App
- Unterstützung von Flight Controllern ohne GNSS
- Behauptung oder Vermarktung als rechtskonformes Direct-Remote-ID-Add-on vor abgeschlossener Konformitätsbewertung
- Verkauf als bereits CE-konformes Endprodukt ohne abgeschlossene Konformitätsbewertung

## 5. Zielnutzer

- Besitzer bestehender FPV-Wings mit INAV
- Besitzer von FPV-Multicoptern mit Betaflight und GNSS
- Modellbauer, die ein kleines Nachrüstmodul suchen
- Open-Source-Entwickler und Hersteller, die eine offene Referenzimplementierung benötigen

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
- Logikpegel UART: 3,3 V
- Verpolungs- und Überspannungsschutz für die Serienhardware
- JST-SH 1.0 oder JST-GH als bevorzugter Stecker
- Lötpads als zusätzliche Anschlussmöglichkeit
- typischer Verbrauch möglichst unter 500 mA
- Masse möglichst unter 10 g ohne Gehäuse und Kabel

### 6.2 Flight-Controller-Protokoll

MVP:

- MSP v2 bevorzugt
- MSP v1 als Fallback
- UART-Baudraten: 115200 Standard, optional 57600
- Polling oder Subscriptions so, dass der FC nicht unnötig belastet wird

Benötigte Daten:

- GNSS-Fix
- Satellitenzahl
- Breitengrad und Längengrad
- absolute beziehungsweise GNSS-Höhe
- Höhe relativ zum Startpunkt
- Geschwindigkeit über Grund
- Kurs über Grund
- Arm-Status
- Zeitstempel, sofern verfügbar

### 6.3 Remote-ID-Ausgabe

Die Firmware soll die OpenDroneID-Core-C-Bibliothek verwenden oder sich eng daran orientieren.

Proof-of-Concept-Ausgabe:

- Bluetooth 4 Legacy Advertising

Nach dem ersten Funktionsnachweis getrennt zu evaluieren:

- Bluetooth 5 Extended/Long Range
- Wi-Fi Beacon
- Wi-Fi NAN

Die Auswahl der später gleichzeitig unterstützten Sendepfade richtet sich nach dem angestrebten regulatorischen Profil, der Plattformunterstützung, der Empfangskompatibilität und den Ergebnissen von Reichweiten- und Interoperabilitätstests.

Zu übertragende Daten:

- UAS-Betreibernummer
- eindeutige Seriennummer des LibreRID-Moduls
- Zeitstempel
- Position des Luftfahrzeugs
- Höhe über Oberfläche oder Startpunkt
- Kurs
- Geschwindigkeit über Grund
- Position des Fernpiloten oder ersatzweise Startpunkt
- Status beziehungsweise Notfallstatus

### 6.4 Verhalten beim Start

1. Modul bootet.
2. Konfiguration und Seriennummer werden geprüft.
3. Verbindung zum Flight Controller wird aufgebaut.
4. Auf gültigen GNSS-Fix wird gewartet.
5. Der Arm-Status des Flight Controllers wird überwacht.
6. Beim Übergang von disarmed zu armed wird bei gültigem GNSS-Fix der Startpunkt unveränderlich für den laufenden Betriebszyklus gespeichert.
7. Ohne gültigen Fix beim Armen wird nicht stillschweigend mit einer alten oder beliebigen Position gesendet; stattdessen wird ein definierter Fehlerzustand signalisiert.
8. Der Broadcast beginnt vor dem Abheben und läuft während des Fluges kontinuierlich.
9. Bei Datenverlust wird ein definierter Fehlerstatus gesendet und über LED angezeigt.

### 6.5 Konfiguration

MVP-Konfiguration vorzugsweise über USB-Massenspeicher oder USB-Weboberfläche.

Beispiel `config.json`:

```json
{
  "operator_id": "TEST-OPERATOR-ID-NOT-FOR-FLIGHT",
  "aircraft_type": "fixed_wing",
  "input_protocol": "msp",
  "uart_baud": 115200,
  "broadcast": {
    "bluetooth_legacy": true,
    "bluetooth_5": false,
    "wifi_beacon": false
  }
}
```

Sicherheitsanforderungen:

- Betreiber-ID nicht im öffentlichen Repository hinterlegen
- Konfiguration nach dem Speichern gegen unbeabsichtigte Änderung schützen
- ungültige oder fehlende Betreiber-ID klar signalisieren
- Möglichkeit zum Zurücksetzen nur durch physischen Tastendruck oder USB-Verbindung

### 6.6 Statusanzeige

Vorschlag:

- Blau blinkend: Start
- Gelb blinkend: Flight Controller nicht erkannt
- Gelb dauerhaft: GNSS noch ohne Fix
- Grün blinkend: gültige Daten, noch nicht armed
- Grün dauerhaft: Remote ID aktiv und vollständig
- Rot blinkend: Konfiguration ungültig
- Rot dauerhaft: schwerer Fehler oder Datenverlust im Flug

## 7. Empfohlene Prototyp-Hardware

### 7.1 Controller

**Festgelegt für Proof of Concept und MVP: ESP32-S3**

Gründe:

- Bluetooth 5 inklusive Extended beziehungsweise Long Range
- Wi-Fi
- ausreichende Rechenleistung
- UART und USB
- bestehende OpenDroneID-Referenzsoftware
- geringe Kosten und gute Verfügbarkeit

Für den ersten Breadboard-Prototyp:

- ESP32-S3-DevKitC-1 beziehungsweise das konkret bestellte ESP32-S3-Entwicklungsboard
- genaue Modul-, Flash- und PSRAM-Variante vor Beginn dokumentieren

Der klassische ESP32 ist für das Projekt nicht erste Wahl, weil Bluetooth 5 Long Range und Extended Advertising fehlen.

### 7.2 Weitere Bauteile

Für den ersten Prototyp:

- ESP32-S3-Devboard
- 4-poliges UART-Kabel
- 470-µF-Pufferkondensator an 5 V
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
│ Flight Controller            │
│ MVP: INAV + GNSS              │
└───────────────┬───────────────┘
                │ MSP über UART
┌───────────────▼───────────────┐
│ Input Adapter                 │
│ MSP v1 / MSP v2               │
└───────────────┬───────────────┘
                │ normalisierte Flugdaten
┌───────────────▼───────────────┐
│ Flight State Manager          │
│ Fix, Home, Armed, Fehler      │
└───────────────┬───────────────┘
                │
┌───────────────▼───────────────┐
│ Identification Encoder       │
│ MVP: EU / OpenDroneID         │
│ Später: FR- und weitere       │
│ internationale Profile       │
└───────────────┬───────────────┘
                │
┌───────────────▼───────────────┐
│ Broadcast Adapter             │
│ PoC: BLE4                     │
│ Später: BLE5, Wi-Fi           │
└───────────────────────────────┘
```

## 9. Abnahmekriterien MVP

Das MVP gilt als technisch erfolgreich, wenn:

1. Ein SpeedyBee F405 Wing Mini mit INAV kann über UART angeschlossen werden.
2. LibreRID erkennt MSP und erhält innerhalb von fünf Sekunden gültige FC-Daten.
3. Die ausgesendeten OpenDroneID-Testdaten werden von einer geeigneten Testanwendung oder einem zweiten Testgerät reproduzierbar empfangen und dekodiert.
4. Position, Höhe, Kurs, Geschwindigkeit, Startpunkt und Seriennummer werden korrekt angezeigt.
5. Der Broadcast beginnt vor dem Abheben und läuft nach dem Armen durchgehend mit den für die ausgewählte Protokollversion konfigurierten Aktualisierungsintervallen.
6. Ein Ausfall der UART-Daten wird innerhalb einer definierten Zeit erkannt und signalisiert.
7. Ein Neustart verliert weder Betreiber-ID noch Modulseriennummer.

Der Empfang in einer App und das Bestehen dieser technischen Kriterien weisen Interoperabilität nach, jedoch keine regulatorische Konformität.

## 10. Testplan

### Labortests

- MSP-Testdaten aus aufgezeichneten INAV-Sessions
- ungültige GNSS-Daten
- wechselnder Fix-Status
- UART-Unterbrechung
- Stromunterbrechung
- falsche Betreiber-ID
- BLE-4-Dauerbroadcast
- Dauerlauf über mindestens zwei Stunden

### Bodentest

- Empfang mit mindestens zwei unterschiedlichen Android-Geräten
- Vergleich der angezeigten Position mit INAV
- Reichweitentest am Boden
- Störtest neben VTX, ELRS und GPS

### Flugtest

- Wing mit INAV
- Copter mit Betaflight erst im nachgelagerten Erweiterungstest
- Startpunkt
- Positionsverlauf
- Datenverlust
- Reichweite
- Verhalten bei Disarm und Landung

## 11. Recht und Konformität

LibreRID soll technisch auf die Anforderungen der EU-Verordnung für Zusatzgeräte zur direkten Fernidentifizierung ausgerichtet werden.

Eine Open-Source-Firmware oder ein funktionsfähiger Prototyp ist jedoch nicht automatisch ein rechtskonformes, CE-gekennzeichnetes Produkt.

Für ein als fertiges Zusatzgerät in Verkehr gebrachtes Produkt sind unter anderem erforderlich:

- technische Dokumentation
- Konformitätsbewertung
- EU-Konformitätserklärung
- CE-Kennzeichnung
- eindeutige physische Seriennummer
- Herstellerangaben und Anleitung
- Prüfung der Betreiber-ID
- Maßnahmen gegen Manipulation
- Einhaltung der Funkanlagen- und EMV-Anforderungen

Das Projekt soll deshalb zwei Ebenen klar unterscheiden:

1. **Open-Source-Referenzimplementierung und Eigenbauprototyp**
2. **Geprüfte LibreRID-Referenzhardware beziehungsweise späteres Serienprodukt**

Bis zur abgeschlossenen Konformitätsbewertung darf das Projekt nicht behaupten, ein Prototyp sei bereits EU-konform. Auch der erfolgreiche Empfang in einer OpenDroneID-App ist nur ein Interoperabilitätsnachweis und kein Konformitätsnachweis.

## 12. Offene Entscheidungen

- genaue ESP32-S3-Board- und Modulvariante für den Prototyp
- ESP32-C3 als mögliche spätere Zielplattform für optimierte Serienhardware
- ESP-IDF oder Arduino/PlatformIO
- Auswahl zusätzlicher Broadcast-Verfahren nach abgeschlossener Einzelbewertung von Bluetooth 5, Wi-Fi Beacon und Wi-Fi NAN
- USB-Massenspeicher versus USB-Webkonfiguration
- genaue MSP-Nachrichten und Aktualisierungsraten
- Ermittlung der Pilotenposition: Startpunkt oder externe Quelle
- Strategie zur Manipulationserschwerung
- Erzeugung und Verwaltung der ANSI/CTA-2063-konformen Seriennummern
- Lizenzmodell für Firmware, Hardware und Dokumentation

## 13. Vorgeschlagene Meilensteine

### M0 – Grundlagen

- Rechts- und Standardanforderungen dokumentieren
- MSP-Daten für INAV bestimmen; Betaflight-Anforderungen für M3 vormerken
- OpenDroneID-Core-C evaluieren

### M1 – Statischer Sender-Demo

- ESP32-S3 sendet statische OpenDroneID-Testdaten über Bluetooth 4 Legacy Advertising
- reproduzierbarer Empfang und Dekodierung
- Build-, Flash- und Testablauf dokumentieren

### M2 – INAV-Anbindung

- MSP über UART
- echte GNSS- und Flugdaten
- SpeedyBee F405 Wing Mini als Referenz
- Startpunkt beim Armen mit gültigem GNSS-Fix erfassen

### M3 – Betaflight-Anbindung

- automatisches Protokoll- und FC-Erkennen
- Copter-Flugtest

### M3a – Weitere Broadcast-Verfahren

- Bluetooth 5 Long Range/Extended Advertising evaluieren
- Wi-Fi Beacon und Wi-Fi NAN getrennt evaluieren
- Empfangskompatibilität, Reichweite und Plattformzuverlässigkeit dokumentieren
- Sendepfade für das angestrebte regulatorische Profil auswählen

### M4 – Konfiguration

- Betreiber-ID
- Seriennummer
- USB-Konfiguration
- Status-LED

### M5 – Referenz-PCB

- Schaltplan
- PCB
- Gehäuse
- BOM und Bauanleitung

### M6 – Konformitäts-Roadmap

- Gap-Analyse
- notwendige Funk-, EMV- und Produkttests
- technische Dokumentation
- Entscheidung über ein marktfähiges Modul

### M7 – Internationale Anforderungsmatrix (künftiges Arbeitspaket)

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


## 14. Projektprinzipien

LibreRID soll nicht als "günstiger DroneTag-Klon" positioniert werden.

Stattdessen verfolgt das Projekt folgende Leitgedanken:

- offene Referenzimplementierung für Direct Remote ID
- einfache Nachrüstung bestehender Modelle
- möglichst keine Änderungen am Flight Controller
- Nutzung vorhandener Sensoren (insbesondere GNSS)
- Plug-and-Play über einen freien UART
- offene Hardware und offene Firmware
- klare Trennung zwischen Referenzimplementierung und später möglichen kommerziellen Produkten

### Designprinzip

Der Idealfall für den Anwender:

1. Modul anschließen (5 V, GND, TX, RX)
2. Freien UART im Flight Controller auf MSP konfigurieren
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

Der Proof of Concept weist zunächst ausschließlich die Erzeugung und den Empfang statischer OpenDroneID-Testdaten über Bluetooth 4 nach. Der erste MVP ergänzt INAV als Flight-Controller-Plattform und MSP als Eingangsschnittstelle. Betaflight, weitere Broadcast-Verfahren und internationale Profile sind nachgelagerte Erweiterungen.

Zielgruppen:

- INAV
- Betaflight
- ArduPilot (später)
- PX4 (später)

Das Projekt soll so aufgebaut werden, dass andere Entwickler eigene Hardware oder Firmware-Erweiterungen entwickeln können, ohne den Kern verändern zu müssen.
