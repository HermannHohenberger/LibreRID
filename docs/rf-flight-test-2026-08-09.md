# Real-flight reception test record — 2026-08-09

## Purpose

Verify live INAV mapping and simultaneous OpenDroneID reception over Bluetooth
4 Legacy and Bluetooth 5 Long Range (LE Coded PHY) during a real fixed-wing
flight.

This is a development test, not a conformity, regulatory range or certification
assessment. The module used a development serial and TEST profile.

## Test setup

| Item | Value |
|---|---|
| Transmitter | Seeed Studio XIAO ESP32-S3 with external 2.4 GHz antenna |
| Firmware transports | BT4 Legacy LE 1M and BT5 Long Range Coded PHY/S=8 preference |
| Configured transmitter power | +9 dBm for both advertising sets |
| Flight controller | INAV, live data over MSP UART |
| Receiver | Android tablet running Drone Scanner |
| Source file | `Drone Scanner Export(2).csv` (not committed; contains identifiers and exact flight path) |
| Source SHA-256 | `d2f2f1cf63a56f1dcea0a5e4580c97f81999b2663ed3330990bc21bb12cb3ae9` |

The scanner export does not contain RSSI or receiver position, so it cannot be
used to determine remaining link margin or a radio range limit.

## Flight and reception results

| Metric | Result |
|---|---:|
| Total recording duration | 767.562 s |
| Detected airborne interval | 659.988 s |
| CSV data rows | 4,314 |
| Unique receive timestamps | 883 |
| Receive events during airborne interval | 771 |
| Airborne receive-event rate | 1.168/s |
| BT4 events during airborne interval | 692 |
| BT5 Long Range events during airborne interval | 79 |
| Median combined airborne reception gap | 0.326 s |
| Longest combined airborne reception gap | 6.114 s |
| Combined airborne gaps over 5 s | 1 |
| Maximum distance from first recorded fix | approximately 489 m |
| Recorded track length | approximately 13.5 km |
| Relative height range while airborne | -5.0 to 93.5 m |
| Maximum horizontal speed | 49.0 m/s |

The scanner decoded Basic ID, Operator ID, Self ID, Location and System Data.
It classified 771 received location messages as `Airborne` and 64 as
`Grounded`. The take-off location remained stable in System Data, and the
dynamic location, altitude, speed and flight status changed plausibly through
the flight.

The longest combined reception gap occurred at approximately 208 m from the
first recorded fix, at 43.5 m relative height and 31 m/s reported horizontal
speed. Other shorter gaps occurred at both near and far positions; this record
therefore does not show a clear hard range boundary.

The small negative relative-height value occurred near the ground and is
consistent with estimator/reference variation. The maximum speed is reported
as encoded by INAV and was not independently verified.

## Privacy-preserving protocol excerpt

The source contains the public operator identifier, device address and exact
flight path. They are intentionally omitted here. Representative decoded data
are summarized without timestamps or coordinates:

```text
Basic ID:    Serial Number / Aeroplane / development serial
Operator ID: present and consistently decoded
Self ID:     LibreRID M1 TEST ONLY
Location:    Grounded and Airborne states with live position, height and speed
System Data: Take Off location with geodetic altitude
Transports:  BluetoothLegacy and BluetoothLongRange
```

## Assessment

Result: **PASS for this development flight test.**

MSP live mapping, arm/disarm-derived flight state, take-off position, dynamic
flight data and both Bluetooth transports operated during the flight. BT4
provided most received events, while BT5 Long Range was independently detected
throughout the recording. Broader repeat flights, controlled receiver placement
and RSSI logging are still required before making a range or reliability claim.
