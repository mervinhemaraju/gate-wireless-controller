#pragma once

// Decodes the CP80 status LED's flash pattern into a gate state, over
// debounced samples. Deliberately takes zero dependency on config.h: all
// timing and polarity come in through StatusDecoderConfig, so this class is
// fully testable with fixture values that don't need to match production
// numbers at all.
//
// Gate state enum mirrors .claude/skills/mqtt-contract/SKILL.md exactly.

#include <cstdint>

enum class GateState { UNKNOWN, OPEN, OPENING, CLOSED, CLOSING, STOPPED, FAULT };

// Single source of truth for the MQTT/log string form of each state.
const char* gateStateToString(GateState s);

// All durations use uint32_t (not unsigned long) deliberately: millis() on
// the real target is a 32-bit value that wraps after ~49.7 days, while
// `unsigned long` on a desktop/native build is often 64-bit. uint32_t keeps
// the wraparound arithmetic identical under `native` tests and on hardware.
struct StatusDecoderConfig {
  uint32_t sampleIntervalMs;
  uint32_t debounceMs;
  uint32_t steadyConfirmMs;
  uint32_t slowFlashMinMs;  // "opening"
  uint32_t slowFlashMaxMs;
  uint32_t fastFlashMinMs;  // "closing"
  uint32_t fastFlashMaxMs;
  uint32_t faultFlashToleranceMs;
  // [UNVERIFIED, Phase 3] Not yet known which way this actually reads on the
  // real gate. See docs/decisions/0002-resistor-divider-over-optocoupler.md.
  bool senseActiveHigh;
};

class StatusDecoder {
public:
  void begin(const StatusDecoderConfig& cfg, uint32_t nowMs);

  // rawSample: the raw physical reading (already polarity-agnostic; true
  // means "pin read high"). Returns true iff currentState() changed on this
  // call. Never blocks waiting for an edge.
  bool tick(bool rawSample, uint32_t nowMs);

  GateState currentState() const;

private:
  StatusDecoderConfig cfg_{};
  GateState state_ = GateState::UNKNOWN;

  bool lastRawLevel_ = false;
  uint32_t lastRawChangeMs_ = 0;
  bool debouncedLevel_ = false;

  bool haveEdge_ = false;
  uint32_t lastEdgeMs_ = 0;
  bool havePeriod_ = false;
  uint32_t lastMeasuredPeriodMs_ = 0;

  uint32_t steadySinceMs_ = 0;

  // Priority order when a measured period could match more than one band:
  // opening/closing are checked before the numbered fault bands. With the
  // current placeholder thresholds in config.h these bands can overlap;
  // this order is the deliberate tiebreak until real timings (Phase 3)
  // replace the placeholders and the overlap question is settled for real.
  GateState classify(uint32_t nowMs) const;
};

#ifdef ARDUINO
// Reads STATUS_SENSE_PIN as an analog sample and threshold-compares it into
// a bool. Kept out of native builds entirely - StatusDecoder itself never
// touches hardware.
bool statusSenseReadRaw();
#endif
