# RF ground range test record — 2026-08-04

## Purpose

Verify simultaneous OpenDroneID reception over Bluetooth 4 Legacy and
Bluetooth 5 Long Range (LE Coded PHY) before increasing transmitter power.

This is a development test, not a conformity or regulatory range assessment.
The module used a development serial and TEST profile.

## Test setup

| Item | Value |
|---|---|
| Transmitter | Seeed Studio XIAO ESP32-S3 with external 2.4 GHz antenna |
| Firmware transports | BT4 Legacy LE 1M and BT5 Long Range Coded PHY/S=8 preference |
| Configured transmitter power | +3 dBm for both advertising sets |
| Transmitter height | approximately 1.60 m above ground |
| Antenna orientation | vertical |
| Receiver | Android tablet running Drone Scanner |
| Maximum reported separation | approximately 210 m |
| RSSI reported by app at maximum separation | approximately -92 dBm |
| Test duration in export | 442.935 s (7 min 22.935 s) |
| Source file | `Drone Scanner Export.csv` (not committed; contains identifiers and exact location) |
| Source SHA-256 | `f37e43db99b462e1b4feab349496b05540cae0aa6e1b2066ff0be6fd6f529934` |

Distance and RSSI were observed manually and are not fields in the CSV export.

## Results

| Metric | Result |
|---|---:|
| CSV data rows | 5,067 |
| Unique receive timestamps | 1,049 |
| BT4 Legacy receive events | 965 |
| BT5 Long Range receive events/message packs | 84 |
| Basic ID rows | 1,049 |
| Self ID rows | 1,048 |
| Operator ID rows | 1,044 |
| System rows | 964 |
| Location rows | 962 |
| Longest gap with no message from either transport | 9.955 s |

Both transports were detected throughout the recording, including near the end
of the test. Every expected OpenDroneID message type was decoded through both
BT4 Legacy and BT5 Long Range. The receiver combined both advertising sets into
one aircraft because they used the same random static address.

Reception weakened substantially during the distant portion. The longest
combined outage was just under ten seconds, with additional outages around
five to seven seconds. Reception at 210 m therefore worked but did not yet have
comfortable continuity margin.

## Sanitized CSV protocol excerpt

The following rows preserve the source columns relevant to transport and
decoding. Exact MAC address, public operator ID and coordinates are redacted in
accordance with the repository privacy policy.

```csv
Message Type,Message Source,Received Timestamp,Mac Address,Status,Latitude,Longitude,Altitude Geodetic (m),Height (m),ID Type,UA Type,UAS ID,Operator ID,Description
Basic ID,BluetoothLegacy,2026-08-04 20:14:03.039,<redacted>,,,,,,Serial Number,Aeroplane,LIBRERIDHH0000000001,,
Basic ID,BluetoothLongRange,2026-08-04 20:14:03.936,<redacted>,,,,,,Serial Number,Aeroplane,LIBRERIDHH0000000001,,
Operator ID,BluetoothLongRange,2026-08-04 20:14:03.936,<redacted>,,,,,,,,,<redacted>,
Location,BluetoothLegacy,2026-08-04 20:14:19.456,<redacted>,Grounded,<redacted>,<redacted>,569.0,0.0,,,,,
Location,BluetoothLongRange,2026-08-04 20:14:20.024,<redacted>,Grounded,<redacted>,<redacted>,568.0,0.0,,,,,
Self ID,BluetoothLegacy,2026-08-04 20:14:03.156,<redacted>,,,,,,,,,,,LibreRID M1 TEST ONLY
```

## Assessment and next test

Result: **PASS with limited margin at maximum separation.**

The next controlled test raises both advertising sets from +3 dBm to +9 dBm.
The same antenna position, orientation, transmitter height, receiver and route
should be retained. Compare:

- RSSI at 10 m and approximately 210 m;
- BT4 and BT5 receive-event counts;
- longest combined reception gap;
- whether BT5 remains visible when BT4 reception becomes intermittent.

The +9 dBm result must be recorded separately rather than overwriting this
baseline.
