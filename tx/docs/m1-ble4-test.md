# Historical M1 BLE4 interoperability test

This record describes the superseded static M1 firmware. The current firmware
does not transmit the Berlin test position or the legacy M1 serial number.

## Purpose

Verify that a second device receives and decodes LibreRID's static OpenDroneID
Bluetooth 4 Legacy Advertising messages. This is an interoperability test, not
a regulatory conformity test.

## Firmware test values

| Field | Expected value |
|---|---|
| Basic ID type | Serial number |
| Serial number | `LIBR0DEV000000000001` |
| Aircraft type | Aeroplane / fixed wing |
| Operator ID | `DE.TEST.LIBRERID` |
| Description | `LibreRID M1 TEST ONLY` |
| Status | Ground |
| Latitude | `52.520000` |
| Longitude | `13.405000` |
| Geodetic altitude | `50 m` |
| Height reference | Takeoff |
| Height | `0 m` |
| Operator location | Takeoff point, same static coordinates |

All values are development placeholders and must not be used for flight.

## Procedure

1. Power the LibreRID board by USB and confirm that the RGB LED is green.
2. Open an OpenDroneID-compatible receiver application on a second device.
3. Grant Bluetooth/nearby-device and location permissions required by Android.
4. Start scanning and wait at least 10 seconds.
5. Select the aircraft with serial number `LIBR0DEV000000000001`.
6. Compare every decoded field with the table above.
7. Power-cycle LibreRID and repeat the scan.
8. Record phone model, OS version, receiver app/version, result and date.

## Pass criteria

- The receiver discovers the transmitter repeatedly.
- Basic ID, Location, Self ID, System and Operator ID are decoded.
- All displayed values match the table above within receiver rounding.
- Reception resumes after a LibreRID power cycle.

## Known scope limit

Bluetooth 4 Legacy Advertising is the M1/MVP demonstration transport. Passing
this test does not demonstrate compliance with EU Direct Remote ID transport or
CE requirements.

## Test record

| Date | Receiver | Result |
|---|---|---|
| 2026-07-20 | Android tablet, OpenDroneID OSM | PASS — transmitter discovered and all expected fields decoded |

Tablet model, Android version and app version remain to be recorded for full
reproducibility.
