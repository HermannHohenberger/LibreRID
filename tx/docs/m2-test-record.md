# M2 identity, validation and persistence test record

Date: 2026-07-20

| Test | Result |
|---|---|
| Fresh/empty configuration rejected | PASS |
| Invalid configuration prevents BLE startup | PASS |
| EASA example `FIN87astrdge12k8-xyz` validates | PASS |
| Modified checksum `FIN87astrdge12k9-xyz` rejected | PASS |
| Explicit `save` required before NVS write | PASS |
| Public M1 test profile saved | PASS |
| Profile survives software restart | PASS |
| Profile survives firmware reflash | PASS |
| BLE resumes after loading valid NVS configuration | PASS |
| Operator ID masked in `show` output | PASS |

The board is intentionally left in the TEST profile with development serial
`LIBRERIDHH0000000001`. A real operator ID may be stored in device NVS but is
not present in this repository. The board is not a conforming production
Remote ID add-on.
