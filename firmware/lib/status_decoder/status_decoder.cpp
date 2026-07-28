#include "status_decoder.h"

#include "logging.h"

#ifdef ARDUINO
#include <Arduino.h>

#include "config.h"
#endif

const char* gateStateToString(GateState s) {
  switch (s) {
    case GateState::UNKNOWN:
      return "unknown";
    case GateState::OPEN:
      return "open";
    case GateState::OPENING:
      return "opening";
    case GateState::CLOSED:
      return "closed";
    case GateState::CLOSING:
      return "closing";
    case GateState::STOPPED:
      return "stopped";
    case GateState::FAULT:
      return "fault";
  }
  return "unknown";
}

void StatusDecoder::begin(const StatusDecoderConfig& cfg, uint32_t nowMs) {
  cfg_ = cfg;
  state_ = GateState::UNKNOWN;

  lastRawLevel_ = false;
  lastRawChangeMs_ = nowMs;
  debouncedLevel_ = false;

  haveEdge_ = false;
  lastEdgeMs_ = nowMs;
  havePeriod_ = false;
  lastMeasuredPeriodMs_ = 0;

  steadySinceMs_ = nowMs;
}

bool StatusDecoder::tick(bool rawSample, uint32_t nowMs) {
  if (rawSample != lastRawLevel_) {
    lastRawChangeMs_ = nowMs;
    lastRawLevel_ = rawSample;
  } else if ((nowMs - lastRawChangeMs_) >= cfg_.debounceMs && debouncedLevel_ != lastRawLevel_) {
    // Accepted (debounced) edge.
    if (haveEdge_) {
      lastMeasuredPeriodMs_ = nowMs - lastEdgeMs_;
      havePeriod_ = true;
    }
    lastEdgeMs_ = nowMs;
    haveEdge_ = true;
    debouncedLevel_ = lastRawLevel_;
    steadySinceMs_ = nowMs;
  }

  GateState newState = classify(nowMs);
  bool changed = (newState != state_);
  if (changed) {
    LOG_INFO("status_decoder: %s -> %s at %lu", gateStateToString(state_),
              gateStateToString(newState), (unsigned long)nowMs);
  }
  state_ = newState;
  return changed;
}

GateState StatusDecoder::currentState() const {
  return state_;
}

GateState StatusDecoder::classify(uint32_t nowMs) const {
  bool lit = cfg_.senseActiveHigh ? debouncedLevel_ : !debouncedLevel_;

  // Unsigned subtraction: wraps correctly across millis()'s ~49.7-day
  // uint32_t rollover, since both operands are uint32_t.
  if ((nowMs - steadySinceMs_) >= cfg_.steadyConfirmMs) {
    return lit ? GateState::OPEN : GateState::CLOSED;
  }

  if (havePeriod_) {
    uint32_t p = lastMeasuredPeriodMs_;

    if (p >= cfg_.slowFlashMinMs && p <= cfg_.slowFlashMaxMs) {
      return GateState::OPENING;
    }
    if (p >= cfg_.fastFlashMinMs && p <= cfg_.fastFlashMaxMs) {
      return GateState::CLOSING;
    }

    // Numbered fault codes are 1-5 flashes/sec [manual p.45].
    for (uint32_t n = 1; n <= 5; ++n) {
      uint32_t faultPeriod = 1000UL / n;
      uint32_t tol = cfg_.faultFlashToleranceMs;
      uint32_t lower = (faultPeriod > tol) ? (faultPeriod - tol) : 0UL;
      uint32_t upper = faultPeriod + tol;
      if (p >= lower && p <= upper) {
        return GateState::FAULT;
      }
    }
  }

  // Matches no configured band: leave state unchanged rather than guess.
  return state_;
}

#ifdef ARDUINO
bool statusSenseReadRaw() {
  return analogRead(STATUS_SENSE_PIN) > STATUS_SENSE_ADC_THRESHOLD_PLACEHOLDER;
}
#endif
