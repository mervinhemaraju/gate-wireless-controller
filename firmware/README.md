# Gate Wireless Controller - Firmware

ESP32 firmware for the on-gate agent described in the repo root
`CLAUDE.md`: pulses a relay across the CP80's `TRG`-`COM` terminals to
trigger the D3 gate motor, reads the CP80 status LED through a resistor
divider to determine gate state, and talks MQTT to the Pi 5 broker per
`.claude/skills/mqtt-contract/SKILL.md`.

## Build / Upload / Monitor

```sh
pio run -e esp32dev              # compile
pio run -e esp32dev -t upload    # flash over USB
pio device monitor -b 115200     # serial log
```

## Tests

```sh
pio test -e native
```

Runs the `gate_control` and `status_decoder` unit tests. No hardware, no
gate, no ESP32 board required - these are the safety-critical modules and
are fully testable as pure state machines.

## Secrets Setup

Copy the example and fill in real values:

```sh
cp include/secrets.h.example include/secrets.h
```

`include/secrets.h` is gitignored. Never commit it. It holds the WiFi PSK,
MQTT broker host/credentials, and the OTA password - everything else
(pins, timing, topics) lives in `include/config.h`, which is not secret and
is committed.

## Before Wiring Anything to the Real CP80

**Bench-test the full firmware with an LED (plus a current-limiting
resistor) standing in for the relay first.** Confirms trigger-pulse timing
and the MQTT round-trip with zero risk to the gate or the CP80 board. Only
move to real wiring after that, and only following
`.claude/skills/esp32-firmware/SKILL.md` and the project's `/wiring-check`
pre-flight checklist. Hard condition 5 in the repo root `CLAUDE.md`: mains
and battery must be disconnected before any wiring work on the motor.

## Known Placeholders (Phase 3 TODOs)

These live in `include/config.h`, clearly marked `[UNVERIFIED, Phase 3]`.
Do not trust the firmware's reported gate state, and do not tune these from
guesswork, until they are replaced by real measurements on the actual gate:

- `STATUS_SENSE_ACTIVE_HIGH_PLACEHOLDER` / `STATUS_SENSE_ADC_THRESHOLD_PLACEHOLDER` -
  the resistor divider's polarity and threshold depend on the CP80 `LED`
  terminal's behaviour under load, not yet measured. See
  [docs/decisions/0002-resistor-divider-over-optocoupler.md](../docs/decisions/0002-resistor-divider-over-optocoupler.md)
  and [docs/wiring/04-gpio-pin-map.md](../docs/wiring/04-gpio-pin-map.md).
- `STATUS_SLOW_FLASH_*`, `STATUS_FAST_FLASH_*`, `STATUS_FAULT_FLASH_TOLERANCE_MS` -
  the manual gives no numeric flash timings for the status LED, only
  qualitative descriptions. See
  [docs/reference/d3-manual-p45-led-indicator-lights.md](../docs/reference/d3-manual-p45-led-indicator-lights.md).
- `TRIGGER_COOLDOWN_MS` - a reasonable placeholder, not tuned against real
  gate cycle timing yet.

`GateState::STOPPED` exists in the enum (per the MQTT contract) but the
current `status_decoder` classifier never produces it - detecting a
gate halted mid-travel from the LED pattern alone needs real data from the
gate first.

## Architecture

Per `.claude/skills/esp32-firmware/SKILL.md`: `src/main.cpp` stays thin
(wiring + dispatch only), one concern per `lib/` module
(`gate_control`, `status_decoder`, `wifi_manager`, `mqtt_client`), no
`delay()` in `loop()`, hardware watchdog fed every iteration, and the pin
map lives only as named constants in `include/config.h`.

`gate_control` and `status_decoder` are fully hardware-agnostic (relay
writes and pin reads are injected/shimmed, not called directly), which is
what makes `pio test -e native` possible without any board attached.
