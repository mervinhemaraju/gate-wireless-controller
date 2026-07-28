#pragma once

// Named constants only. No business logic lives here. Every GPIO number in
// the project must resolve through this file rather than appear inline
// elsewhere (.claude/skills/esp32-firmware/SKILL.md).
//
// This header is included by lib/gate_control (which must also compile
// under the hardware-free `native` test environment), so it must not depend
// on anything Arduino.h would otherwise supply - hence <cstdint> below, and
// plain integers instead of the HIGH/LOW macros.

#include <cstddef>
#include <cstdint>

// ---- Pin Map (docs/wiring/04-gpio-pin-map.md) ----
constexpr int RELAY_TRIGGER_PIN = 27; // Output. Avoids strapping pins (0,2,5,12,15),
                                       // flash pins (6-11), input-only pins (34-39),
                                       // and UART0 (1,3).
constexpr int STATUS_SENSE_PIN  = 34; // Input-only, ADC1-capable. No pull-up needed:
                                       // the resistor divider actively drives the line.

// ---- Relay Polarity & Trigger Timing ----
// Measured 2026-07-28 on the bench (docs/wiring/02-relay-module.md): this
// board is active-HIGH in its current jumper position, the opposite of the
// generic "opto-isolated relays are usually active-low" assumption. Do not
// flip these without re-running that bench test.
constexpr int RELAY_ACTIVE_LEVEL = 1; // Arduino HIGH
constexpr int RELAY_IDLE_LEVEL   = 0; // Arduino LOW

constexpr uint32_t TRIGGER_PULSE_MS    = 500;
constexpr uint32_t TRIGGER_COOLDOWN_MS = 3000; // placeholder; tune once real gate timing is known

constexpr size_t REQUEST_ID_MAX_LEN      = 40; // covers a 36-char UUID + null, with margin
constexpr size_t REQUEST_ID_HISTORY_SIZE = 16; // ring buffer of recently-seen request_ids

// ---- Status Sense (docs/decisions/0002-resistor-divider-over-optocoupler.md,
// docs/wiring/04-gpio-pin-map.md) ----
// [UNVERIFIED, Phase 3] Whether a HIGH reading on STATUS_SENSE_PIN means the
// CP80 LED terminal is lit is NOT yet known - it depends on the terminal's
// behaviour under load, which has to be measured on the real gate. Do not
// treat these as measured facts; they are placeholders so the firmware
// compiles and is testable before that measurement exists.
constexpr bool STATUS_SENSE_ACTIVE_HIGH_PLACEHOLDER   = true;
constexpr int  STATUS_SENSE_ADC_THRESHOLD_PLACEHOLDER = 2048; // out of 0-4095, unmeasured

// ---- Status Decoder Timing ----
// All placeholders. The manual (docs/reference/d3-manual-p45-led-indicator-lights.md)
// gives no numeric flash timings, only qualitative descriptions (off=closed,
// on=open, slow flash=opening, fast flash=closing, 1-5 flashes/sec=faults).
// Real timings are a Phase 3 measurement against the actual gate.
constexpr uint32_t STATUS_SAMPLE_INTERVAL_MS       = 50;
constexpr uint32_t STATUS_DEBOUNCE_MS              = 50;
constexpr uint32_t STATUS_STEADY_CONFIRM_MS        = 2000;
constexpr uint32_t STATUS_SLOW_FLASH_MIN_MS        = 800;  // "opening"
constexpr uint32_t STATUS_SLOW_FLASH_MAX_MS        = 1400;
constexpr uint32_t STATUS_FAST_FLASH_MIN_MS        = 300;  // "closing"
constexpr uint32_t STATUS_FAST_FLASH_MAX_MS        = 600;
constexpr uint32_t STATUS_FAULT_FLASH_TOLERANCE_MS = 80;
// Fault codes are numbered flash rates, 1-5 Hz [manual p.45]. All five map to
// GateState::FAULT - the state enum has no fault subcode, and distinguishing
// "no mains" from "battery low" etc. is not needed for this system's purpose.

// ---- Networking ----
constexpr uint32_t WIFI_BACKOFF_INITIAL_MS = 1000;
constexpr uint32_t WIFI_BACKOFF_MAX_MS     = 60000;
constexpr uint32_t MQTT_BACKOFF_INITIAL_MS = 1000;
constexpr uint32_t MQTT_BACKOFF_MAX_MS     = 60000;
constexpr uint16_t MQTT_KEEPALIVE_S = 30;
constexpr const char* MQTT_CLIENT_ID = "gate-esp32";

// ---- MQTT Topics & Schema (.claude/skills/mqtt-contract/SKILL.md) ----
constexpr const char* TOPIC_AVAILABILITY = "gate/availability";
constexpr const char* TOPIC_STATE        = "gate/state";
constexpr const char* TOPIC_CMD_TRIGGER  = "gate/cmd/trigger";
constexpr const char* TOPIC_CMD_ACK      = "gate/cmd/ack";
constexpr int SCHEMA_VERSION = 1;
constexpr uint32_t STATE_HEARTBEAT_INTERVAL_MS = 60000;

// ---- Watchdog ----
constexpr uint32_t WDT_TIMEOUT_S = 10;

// ---- OTA ----
constexpr const char* OTA_HOSTNAME = "gate-esp32";
