#include "gate_control.h"

#include <cstring>

#include "logging.h"

#ifdef ARDUINO
#include <Arduino.h>
#endif

const char* gateControlResultToString(GateControlResult r) {
  switch (r) {
    case GateControlResult::ACCEPTED:
      return "accepted";
    case GateControlResult::DUPLICATE:
      return "duplicate";
    case GateControlResult::REJECTED_COOLDOWN:
      return "rejected_cooldown";
    case GateControlResult::REJECTED_BUSY:
      return "rejected_busy";
    case GateControlResult::REJECTED_MALFORMED_ID:
      return "rejected_malformed";
  }
  return "unknown";
}

void GateControl::begin(PinWriteFn writeRelay, uint32_t nowMs) {
  writeRelay_ = writeRelay;
  state_ = PulseState::IDLE;
  pulseStartMs_ = nowMs;
  cooldownStartMs_ = nowMs;
  seenHead_ = 0;
  seenCount_ = 0;
  for (size_t i = 0; i < REQUEST_ID_HISTORY_SIZE; ++i) {
    seenIds_[i][0] = '\0';
  }
  if (writeRelay_ != nullptr) {
    writeRelay_(RELAY_IDLE_LEVEL);
  }
}

bool GateControl::isDuplicate(const char* requestId) const {
  for (size_t i = 0; i < seenCount_; ++i) {
    if (strncmp(seenIds_[i], requestId, REQUEST_ID_MAX_LEN) == 0) {
      return true;
    }
  }
  return false;
}

void GateControl::recordSeen(const char* requestId) {
  strncpy(seenIds_[seenHead_], requestId, REQUEST_ID_MAX_LEN - 1);
  seenIds_[seenHead_][REQUEST_ID_MAX_LEN - 1] = '\0';
  seenHead_ = (seenHead_ + 1) % REQUEST_ID_HISTORY_SIZE;
  if (seenCount_ < REQUEST_ID_HISTORY_SIZE) {
    seenCount_++;
  }
}

GateControlResult GateControl::requestTrigger(const char* requestId, uint32_t nowMs) {
  if (requestId == nullptr || strnlen(requestId, REQUEST_ID_MAX_LEN) >= REQUEST_ID_MAX_LEN) {
    LOG_WARN("gate_control: rejected malformed request_id");
    return GateControlResult::REJECTED_MALFORMED_ID;
  }

  if (isDuplicate(requestId)) {
    LOG_INFO("gate_control: duplicate request_id=%s", requestId);
    return GateControlResult::DUPLICATE;
  }

  // Recorded unconditionally, before the state branch below. This is what
  // makes dedup outlive cooldown: a redelivered id must be rejected even if
  // it arrives long after the original pulse and cooldown have finished.
  recordSeen(requestId);

  switch (state_) {
    case PulseState::IDLE: {
      pulseStartMs_ = nowMs;
      state_ = PulseState::PULSING;
      if (writeRelay_ != nullptr) {
        writeRelay_(RELAY_ACTIVE_LEVEL);
      }
      LOG_INFO("gate_control: trigger accepted request_id=%s at %lu", requestId,
                (unsigned long)nowMs);
      return GateControlResult::ACCEPTED;
    }
    case PulseState::PULSING:
      LOG_INFO("gate_control: trigger rejected (busy) request_id=%s", requestId);
      return GateControlResult::REJECTED_BUSY;
    case PulseState::COOLDOWN:
      LOG_INFO("gate_control: trigger rejected (cooldown) request_id=%s", requestId);
      return GateControlResult::REJECTED_COOLDOWN;
  }
  return GateControlResult::REJECTED_MALFORMED_ID; // unreachable, silences -Wreturn-type
}

void GateControl::tick(uint32_t nowMs) {
  switch (state_) {
    case PulseState::PULSING:
      // Unsigned subtraction: wraps correctly across millis()'s ~49.7-day
      // uint32_t rollover, since both operands are uint32_t.
      if ((nowMs - pulseStartMs_) >= TRIGGER_PULSE_MS) {
        if (writeRelay_ != nullptr) {
          writeRelay_(RELAY_IDLE_LEVEL);
        }
        cooldownStartMs_ = nowMs;
        state_ = PulseState::COOLDOWN;
        LOG_INFO("gate_control: pulse ended, cooldown started at %lu", (unsigned long)nowMs);
      }
      break;
    case PulseState::COOLDOWN:
      if ((nowMs - cooldownStartMs_) >= TRIGGER_COOLDOWN_MS) {
        state_ = PulseState::IDLE;
        LOG_INFO("gate_control: cooldown ended at %lu", (unsigned long)nowMs);
      }
      break;
    case PulseState::IDLE:
      break;
  }
}

bool GateControl::isPulsing() const {
  return state_ == PulseState::PULSING;
}

#ifdef ARDUINO
void relayPinWrite(int level) {
  digitalWrite(RELAY_TRIGGER_PIN, level);
}

void gateControlHardwareInit() {
  // Boot safety: the level must be latched BEFORE pinMode(OUTPUT) runs, or
  // the pin floats through an undefined state during the mode change and
  // can glitch the relay. See .claude/skills/esp32-firmware/SKILL.md.
  digitalWrite(RELAY_TRIGGER_PIN, RELAY_IDLE_LEVEL);
  pinMode(RELAY_TRIGGER_PIN, OUTPUT);
  digitalWrite(RELAY_TRIGGER_PIN, RELAY_IDLE_LEVEL);
}
#endif
