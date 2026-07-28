#pragma once

// Owns the trigger pulse, cooldown, and request_id dedup for the relay that
// pulses the CP80's TRG-COM terminals. This is the safety-critical module:
// firmware is the last line of defence against a stuck-closed relay or a
// replayed command, regardless of what the network sends.
//
// Fully hardware-agnostic: relay writes go through an injected function
// pointer, so this compiles and is unit-testable under PlatformIO's
// `native` environment with no Arduino dependency and no gate involved.

#include <cstddef>
#include <cstdint>

#include "config.h"

enum class GateControlResult {
  ACCEPTED,
  DUPLICATE,
  REJECTED_COOLDOWN,
  REJECTED_BUSY,
  REJECTED_MALFORMED_ID
};

// Matches the gate/cmd/ack `result` values 1:1
// (.claude/skills/mqtt-contract/SKILL.md).
const char* gateControlResultToString(GateControlResult r);

class GateControl {
public:
  using PinWriteFn = void (*)(int level);

  // writeRelay: injected pin-write function. In production this wraps
  // digitalWrite(RELAY_TRIGGER_PIN, level); tests pass a spy instead.
  // Immediately drives the relay to its idle level.
  void begin(PinWriteFn writeRelay, uint32_t nowMs);

  // Call every loop iteration. Advances the pulse/cooldown state machine.
  // Never blocks; all timing is nowMs-driven.
  void tick(uint32_t nowMs);

  // requestId must be a NUL-terminated string shorter than
  // REQUEST_ID_MAX_LEN. Duplicate ids are rejected regardless of the
  // controller's current state - dedup outlives cooldown, since QoS 1
  // redelivery can arrive well after a pulse has already completed.
  GateControlResult requestTrigger(const char* requestId, uint32_t nowMs);

  bool isPulsing() const;

private:
  enum class PulseState { IDLE, PULSING, COOLDOWN };

  PinWriteFn writeRelay_ = nullptr;
  PulseState state_ = PulseState::IDLE;
  // uint32_t (not unsigned long) deliberately: on the real target,
  // millis() returns a 32-bit value that wraps after ~49.7 days, and
  // `unsigned long` on a desktop/native build is often 64-bit. Using
  // uint32_t explicitly here makes the wraparound arithmetic below behave
  // identically under `native` tests and on real hardware.
  uint32_t pulseStartMs_ = 0;
  uint32_t cooldownStartMs_ = 0;

  char seenIds_[REQUEST_ID_HISTORY_SIZE][REQUEST_ID_MAX_LEN];
  size_t seenHead_ = 0;
  size_t seenCount_ = 0;

  bool isDuplicate(const char* requestId) const;
  void recordSeen(const char* requestId);
};

#ifdef ARDUINO
// Boot-safety sequence. Call as the very first line of setup(), before
// GateControl::begin() and before anything else touches the pin. Latches
// the idle level BEFORE pinMode(OUTPUT) runs, so the pin never floats
// through an undefined state during the mode change.
void gateControlHardwareInit();

// Concrete PinWriteFn passed to GateControl::begin() on real hardware.
void relayPinWrite(int level);
#endif
