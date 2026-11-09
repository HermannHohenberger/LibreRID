# RF ground range test record — 2026-08-05

## Purpose

Repeat the 2026-08-04 ground test after raising both Bluetooth advertising
sets from +3 dBm to +9 dBm. The +3 dBm baseline is documented in
[`rf-range-test-2026-08-04.md`](rf-range-test-2026-08-04.md).

This is a development test, not a conformity or regulatory range assessment.

## Test setup

| Item | Value |
|---|---|
| Transmitter | Seeed Studio XIAO ESP32-S3 with external 2.4 GHz antenna |
| Firmware transports | BT4 Legacy LE 1M and BT5 Long Range Coded PHY/S=8 preference |
| Configured transmitter power | +9 dBm for both advertising sets |
| Receiver | Android tablet running Drone Scanner |
| Test duration in export | 472.881 s (7 min 52.881 s) |
| Source file | `Drone Scanner Export(1).csv` (not committed; contains identifiers and exact location) |
| Source SHA-256 | `ad8f9a6f11dccb9ba392ea0b6ce39600c9c98741f55c44a99b59e1d7c1e51cc8` |

RSSI and receiver distance are not fields in the CSV export. The operator's
qualitative observation was that there was no large improvement over +3 dBm.

## Results

| Metric | +3 dBm baseline | +9 dBm test |
|---|---:|---:|
| Duration | 442.935 s | 472.881 s |
| CSV data rows | 5,067 | 2,185 |
| Unique receive timestamps | 1,049 | 437 |
| BT4 Legacy receive events | 965 | 394 |
| BT5 Long Range receive events/message packs | 84 | 43 |
| All-transport event rate | 2.368/s | 0.924/s |
| BT4 event rate | 2.179/s | 0.833/s |
| BT5 event rate | 0.190/s | 0.091/s |
| Median combined gap | 0.207 s | 0.323 s |
| Longest combined gap | 9.955 s | 79.335 s |
| Combined gaps over 2 s | 46 | 64 |
| Combined gaps over 5 s | 8 | 9 |
| Combined gaps over 10 s | 0 | 4 |

All five expected OpenDroneID message types were decoded over both BT4 Legacy
and BT5 Long Range. Identity data remained consistent and one transmitter was
observed. The dual-transport firmware therefore continued to function at
+9 dBm.

The measured reception continuity did not improve. The export contains a
79.335-second interval without any BT4 or BT5 message, from 19:56:07.258 to
19:57:26.593. Two consecutive 30-second bins in the middle of the recording
contain no receive event at all. This is materially worse than the baseline,
but the CSV cannot distinguish among radio loss, receiver scanning pauses,
route/orientation differences or other environmental changes.

## Sanitized CSV protocol excerpt

```csv
Message Type,Message Source,Received Timestamp,Mac Address,Status,Latitude,Longitude,Altitude Geodetic (m),Height (m),ID Type,UA Type,UAS ID,Operator ID,Description
Location,BluetoothLegacy,2026-08-05 19:52:37.609,<redacted>,Grounded,<redacted>,<redacted>,567.0,0.0,,,,,
Basic ID,BluetoothLegacy,2026-08-05 19:52:37.609,<redacted>,,,,,,Serial Number,Aeroplane,LIBRERIDHH0000000001,,
Operator ID,BluetoothLegacy,2026-08-05 19:52:37.609,<redacted>,,,,,,,,,<redacted>,
Self ID,BluetoothLegacy,2026-08-05 19:52:37.609,<redacted>,,,,,,,,,,,LibreRID M1 TEST ONLY
```

## Assessment

Result: **Dual transport PASS; +9 dBm range improvement not demonstrated.**

The two recordings are useful engineering observations but are not a controlled
power comparison because the CSV contains neither receiver position nor RSSI.
The unusually long total blackout in the +9 dBm recording dominates aggregate
counts and prevents attributing the result to transmitter power alone.

Recommended next step: retain +9 dBm, test at fixed marked distances, keep the
tablet screen and scanner active, hold antenna orientations constant, manually
record RSSI at each point, and wait a fixed interval such as 30 seconds at each
distance. A second receiver running in parallel would help distinguish an
Android scan pause from a transmitter-side outage.
