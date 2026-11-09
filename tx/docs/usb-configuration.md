# USB configuration

LibreRID M2 stores identity data in the ESP32-S3 non-volatile storage (NVS).
Configuration is staged in RAM, validated, and only persisted by the explicit
`save` command. A successful save restarts the module.

## Open the console

```sh
pio device monitor --baud 115200
```

Enter `help` to list commands and `show` to display status. The operator ID is
masked in console output.

## Configure real identity data

```text
set operator CCCxxxxxxxxxxxxc-sss
set serial XXXXXXXXXXXXXXXXXXXX
show
save
```

The operator value is the complete EU registration string supplied by the
registration authority: three-letter country code, twelve lowercase
alphanumeric registration characters, checksum, hyphen, and three secret
characters. LibreRID verifies the Luhn-mod-36 checksum before saving.

The MVP serial validator currently requires exactly 20 alphanumeric characters.
Formal ANSI/CTA-2063 manufacturer-code allocation is still unresolved; do not
invent a production serial number.

Never add a real operator ID to source files, documentation, bug reports or
version control.

## Development profile

```text
test
save
```

This persists the public M1 test identifiers. The profile is visibly marked as
test-only and must not be used for flight.

For a bench test with a real EU operator ID but without an allocated LibreRID
CTA-2063 serial number, first stage the public test profile and then replace only
the operator ID:

```text
test
set operator CCCxxxxxxxxxxxxc-sss
show
save
```

The module remains a development profile because its serial number is the
obvious `LIBRERIDHH0000000001` placeholder. It must not be presented as a
conforming add-on.

The complete registration string, including the three secret characters, is
used only for local validation. Bluetooth messages contain only the
16-character public operator ID.

## Reset

With a physical USB connection:

```text
reset
```

This erases the identity configuration and restarts LibreRID. It does not erase
or replace the firmware.

## LED states implemented in M2

| LED | Meaning |
|---|---|
| Blue blinking | Booting |
| Red blinking | Missing or invalid identity configuration |
| Amber solid | Flight controller or valid GNSS data missing |
| Green solid | Live data valid / BLE4 broadcast active |
| Red solid | Fatal firmware or BLE error |
