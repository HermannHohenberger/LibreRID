# M4 INAV UART bring-up

LibreRID uses a dedicated full-duplex UART at 115200 baud. The development
firmware starts the MSP probe and dynamic OpenDroneID mapping automatically at
boot. Console commands remain available for diagnostics.

## ESP32-S3 pins

| ESP32-S3 board | Function | Flight controller |
|---|---|---|
| GPIO 18 | LibreRID RX | TX of the selected FC UART |
| GPIO 17 | LibreRID TX | RX of the selected FC UART |
| GND | Common ground | GND |

### Seeed Studio XIAO ESP32-S3

| XIAO pad | Chip pin / function | Flight controller |
|---|---|---|
| D7 / RX | GPIO44, LibreRID RX | TX of the selected FC UART |
| D6 / TX | GPIO43, LibreRID TX | RX of the selected FC UART |
| GND | Common ground | GND |
| 5V | Protected 5 V input | FC regulated 5 V through Schottky diode |

Seeed requires a diode when an external source powers the XIAO through its 5V
pad: diode anode to FC 5 V, cathode (striped end) to the XIAO 5V pad. Do not
connect FC power and USB power simultaneously. The external 2.4 GHz antenna must
be attached to the U.FL connector before powering the board.

TX and RX are crossed. Both sides use 3.3 V UART logic.

During the first bench test, power both boards through their own USB connectors
and connect only GND, TX and RX. Do not connect the 5 V rails while both USB
power sources are attached. Later standalone tests may power LibreRID from the
FC 5 V rail after USB has been disconnected.

## INAV configuration

In INAV Configurator:

1. Open **Ports**.
2. Select one otherwise unused hardware UART.
3. Enable **MSP** for that UART.
4. Select `115200` baud for MSP.
5. Set Telemetry Output to **Disabled**. In particular, do not enable MAVLink on
   the same UART: INAV activates it on ARM and it replaces/corrupts MSP traffic.
6. Keep Serial RX off and Sensor Input/Peripherals disabled.
7. Save and reboot the flight controller.

The UART must not be shared with GNSS, an OSD, a receiver or other telemetry in
the LibreRID MVP.

## Probe

After boot, wait five seconds and enter:

```text
msp show
```

A successful connection reports `fc=yes`, `inav=yes`, the INAV MSP API version,
GPS fields, relative altitude and an independently mapped arm state. The client
starts with native MSPv2 and falls back to MSPv1 after two seconds without any
valid response.

`msp start` and `msp broadcast` remain available for manual diagnostics, but
normal startup no longer requires either command.

```text
msp broadcast
```

Manual `msp broadcast` is deliberately rejected outside the test profile.

## Polling rates

| Data | MSP command | Target rate |
|---|---|---|
| Identity/API | `MSP_API_VERSION`, `MSP_FC_VARIANT` | discovery only |
| GNSS | `MSP_RAW_GPS` | 5 Hz |
| Arm state | `MSP_BOXIDS`, `MSP_ACTIVEBOXES` | map once / 5 Hz |
| Relative altitude | `MSP_ALTITUDE` | 2 Hz |
