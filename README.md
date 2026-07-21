# LibreRID

Open Electronic Identification Module prototype for FPV-Drones.

LibreRID reads live GNSS, altitude and ARM state from an INAV flight controller
over MSP and broadcasts OpenDroneID messages using Bluetooth 4 Legacy
Advertising. MSP discovery and live mapping start automatically after boot.

This repository contains experimental firmware, not a certified EU Direct
Remote Identification add-on. The current development serial number and TEST
profile must not be used as evidence of regulatory conformity.

## Development hardware

- ESP32-S3-N16R8 development board
- 16 MB flash
- 8 MB PSRAM
- native USB-C connector
- onboard addressable RGB LED, expected on GPIO 48
- SpeedyBee F405 Wing Mini running INAV 7.x
- dedicated UART4 at 115200 baud

UART wiring:

| SpeedyBee | ESP32-S3 |
|---|---|
| T4 | GPIO18 (RX) |
| R4 | GPIO17 (TX) |
| GND | GND |

UART4 must have MSP enabled at 115200 baud. Telemetry Output, including
MAVLink, Serial RX, Sensor Input and Peripherals must be disabled on UART4.

## Build

```sh
pio run
```

## Flash and monitor

Connect the USB-C socket labeled `USB` on the board. If the board is not
detected automatically, hold `BOOT`, tap `RST`, then release `BOOT`.

```sh
pio run --target upload
pio device monitor
```

Expected serial output includes `LibreRID MSP diagnostic build`, flash and
PSRAM sizes, self-test results and automatic MSP/live-mapping startup.

Green indicates valid live data or active broadcasting. Amber indicates missing
FC/GNSS data. Red blinking indicates invalid configuration and red solid a
fatal firmware or BLE error.

Identity configuration and NVS persistence are documented in
[`docs/usb-configuration.md`](docs/usb-configuration.md).

## Identity and privacy

The current development serial is `LIBRERIDHH0000000001`. A complete EU
operator registration string can be entered locally over USB and is stored only
in ESP32 NVS. LibreRID validates the complete string but broadcasts only its
16-character public part; the three secret characters are never intentionally
broadcast or written to logs.

No real operator ID or test location is stored in this repository. Location and
System messages are not sent until valid live INAV data is available.

Before publishing logs, screenshots or bug reports, remove exact coordinates,
public operator IDs and device identifiers.

## Reference encoder

`lib/opendroneid` vendors `opendroneid-core-c` at commit
`4b266c7c33e5299bfbe8427ed8518e869e3a7d7f` under Apache-2.0.

## License

LibreRID is licensed under the Apache License 2.0. See [`LICENSE`](LICENSE).

## Current limitations

- development serial is not an allocated ANSI/CTA-2063 manufacturer serial;
- no EU declaration of conformity or radio/conformity assessment;
- no cryptographic authentication (`Untrusted` in some receiver apps);
- Bluetooth 4 Legacy is the only implemented broadcast transport;
- real-flight validation and RF range testing remain outstanding.
