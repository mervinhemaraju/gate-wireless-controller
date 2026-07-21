---
name: esp32-firmware
description: Conventions and safety rules for the ESP32 firmware in this project. Load BEFORE writing, editing or reviewing anything under firmware/, before choosing GPIO pins, before touching the relay trigger or status LED decoding, and before setting up platformio.ini. Invoke with /esp32-firmware [what you are working on].
---

# ESP32 Firmware Rules: $ARGUMENTS

Applies to everything under `firmware/`. The board is a 30-pin ESP32
(CP2102 TYPE-C) sitting inside the D3 motor housing, driving an opto-isolated
relay on TRG-COM and reading the CP80 status LED through a PC817.

Follow every section below for the work described in `$ARGUMENTS`. These are
requirements, not suggestions: this device physically moves a heavy gate.

This device can physically move a heavy sliding gate. Treat every defect as a
safety defect, not a cosmetic one.

## Toolchain

- PlatformIO, not the Arduino IDE. The build must be reproducible from
  `platformio.ini` alone
- Pin the platform and every library to an exact version. No floating refs,
  same principle as the Terraform and Docker rules

```ini
[env:esp32dev]
platform = espressif32@6.5.0
board = esp32dev
framework = arduino
monitor_speed = 115200
lib_deps =
    knolleary/PubSubClient@2.8
    bblanchon/ArduinoJson@7.0.4
```

- `build_flags` carry compile-time config; never edit a library in place

## Structure

- `src/main.cpp` stays thin: setup, wiring up modules, and a loop that only
  dispatches
- One concern per module, header plus implementation:
  `wifi_manager`, `mqtt_client`, `gate_control`, `status_decoder`, `config`
- No business logic in `loop()` and none in interrupt handlers
- The full pin map lives in exactly one header (`config.h`) as named
  constants. Never a bare GPIO number inline

## Main Loop

- **Never call `delay()` in `loop()`.** A blocking delay is a missed MQTT
  keepalive and a dropped gate command
- Time with `millis()` deltas, and always compare as
  `(now - last) >= INTERVAL` so the 49-day rollover is handled correctly
- Every module exposes a non-blocking `tick()`; `loop()` calls them in order
- ISRs are minimal: set a `volatile` flag, do the work in `tick()`. Any
  variable shared with an ISR is `volatile`

## Watchdog and Recovery

- Hardware watchdog enabled and fed from `loop()`, always. This box is inside
  a motor housing and a manual power cycle means walking out to the gate
- Brownout detector left enabled. The 12V aux rail sags when the motor starts,
  and a silent reset mid-pulse is worse than a clean one
- WiFi and MQTT reconnect on exponential backoff (1s doubling to a 60s cap,
  with jitter), never a tight retry loop
- Reconnection must never re-fire a pending command. State is rebuilt from the
  hardware, never replayed from the network
- OTA updates (ArduinoOTA) are worth having precisely because the board is
  hard to reach; gate it behind a password from `secrets.h`

## GPIO and Actuation Safety

- Drive the relay pin to its inactive level **before** setting `pinMode`
  OUTPUT. Setting the mode first leaves a window where the pin floats and can
  glitch the gate open at boot
- The relay modules in this build are opto-isolated and typically active-low,
  so idle is usually HIGH. Confirm the polarity on the bench with the gate
  disconnected before trusting it, and record the result in `config.h`
- The PC817 output pulls the GPIO **low** when the status LED is lit. Use
  `INPUT_PULLUP` and treat the signal as active-low
- Avoid the strapping pins (GPIO 0, 2, 5, 12, 15) for the relay or any line
  that must be quiet at boot. GPIO 6-11 are wired to flash and unusable.
  GPIO 34-39 are input-only with no internal pullups, so they cannot drive the
  relay and cannot be used for the pullup-dependent optocoupler input
- The trigger pulse is bounded in firmware (`TRIGGER_PULSE_MS`, around 500ms)
  and is single-shot. Firmware is the last line of defence: no API bug, broker
  replay, or malformed payload may hold TRG closed
- Enforce a cooldown between pulses and reject overlapping trigger requests.
  Two pulses in quick succession means stop-then-reverse on a moving gate
- Decode the status LED flash pattern with an explicit state machine over
  debounced samples. Never block waiting for an edge

## Secrets

- WiFi PSK, MQTT credentials and the OTA password live in `firmware/secrets.h`
  (gitignored) or in `build_flags` sourced from the environment
- Commit a `secrets.h.example` with placeholder values so the build is
  documented
- Never log credentials, and never publish them to an MQTT topic

## Logging

- Leveled logging macros, verbose output compiled out of release builds
- Log state transitions and every trigger pulse with a timestamp. When the
  gate misbehaves, this log is the only witness

## Testing

- The status decoder and the trigger cooldown logic are pure functions of time
  and input. Test them under PlatformIO's `native` environment, no hardware
  needed
- Bench-test the full firmware with an LED standing in for the relay before
  wiring anything to the CP80, per hard condition 5
