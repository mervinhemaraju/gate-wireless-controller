// Native, hardware-free tests for status_decoder. Run with: pio test -e native
//
// StatusDecoder takes zero dependency on config.h, so every test builds its
// own fixture config with values chosen for test clarity - they do not need
// to (and deliberately do not) match production numbers.

#include <unity.h>

#include <cstdint>

#include "status_decoder.h"

namespace {

StatusDecoderConfig makeFixtureConfig(bool activeHigh) {
  StatusDecoderConfig cfg{};
  cfg.sampleIntervalMs = 10;
  cfg.debounceMs = 20;
  cfg.steadyConfirmMs = 500;
  cfg.slowFlashMinMs = 200; // "opening"
  cfg.slowFlashMaxMs = 300;
  cfg.fastFlashMinMs = 50; // "closing"
  cfg.fastFlashMaxMs = 100;
  cfg.faultFlashToleranceMs = 5;
  cfg.senseActiveHigh = activeHigh;
  return cfg;
}

// Drives exactly one measured period (two accepted debounced edges,
// periodMs apart) starting from startT, and returns the resulting
// classification read immediately after the second edge settles.
//
// The first edge is accepted at startT + debounceMs. To make the SECOND
// edge accepted exactly periodMs after that (not periodMs plus a second
// debounce delay), the second raw flip happens at
// startT + periodMs - debounceMs, not startT + periodMs. Requires
// periodMs > debounceMs, true for every caller below.
GateState feedOnePeriod(StatusDecoder& d, uint32_t periodMs, uint32_t debounceMs,
                          uint32_t startT) {
  uint32_t t = startT;
  bool level = true;

  d.tick(level, t); // first raw change
  t += debounceMs;
  d.tick(level, t); // first accepted edge at startT + debounceMs

  t += (periodMs - debounceMs);
  level = !level;
  d.tick(level, t); // second raw change
  t += debounceMs;
  d.tick(level, t); // second accepted edge -> period == periodMs, exactly

  return d.currentState();
}

} // namespace

void setUp(void) {}
void tearDown(void) {}

// 1. Before any samples, currentState() is UNKNOWN, never CLOSED. A wrong
//    "closed" is the one lie this system must never tell
//    (.claude/skills/mqtt-contract/SKILL.md).
void test_starts_unknown_not_closed(void) {
  StatusDecoder d;
  StatusDecoderConfig cfg = makeFixtureConfig(true);
  d.begin(cfg, 0);

  TEST_ASSERT_EQUAL(GateState::UNKNOWN, d.currentState());
}

// 2. Raw flicker faster than debounceMs never produces an accepted edge or
//    a state change.
void test_subdebounce_flicker_ignored(void) {
  StatusDecoder d;
  StatusDecoderConfig cfg = makeFixtureConfig(true);
  d.begin(cfg, 0);

  uint32_t t = 0;
  bool level = false;
  for (int i = 0; i < 50; ++i) {
    t += 5; // well under debounceMs = 20
    level = !level;
    bool changed = d.tick(level, t);
    TEST_ASSERT_FALSE(changed);
  }
  TEST_ASSERT_EQUAL(GateState::UNKNOWN, d.currentState());
}

// 3. A steady debounced HIGH, held past steadyConfirmMs, with
//    senseActiveHigh = true, decodes to OPEN.
void test_steady_high_is_open_when_active_high(void) {
  StatusDecoder d;
  StatusDecoderConfig cfg = makeFixtureConfig(true);
  d.begin(cfg, 0);

  uint32_t t = 0;
  d.tick(true, t);
  t += cfg.debounceMs;
  d.tick(true, t); // accepted edge -> debouncedLevel_ = true
  t += cfg.steadyConfirmMs;
  bool changed = d.tick(true, t);

  TEST_ASSERT_TRUE(changed);
  TEST_ASSERT_EQUAL(GateState::OPEN, d.currentState());
}

// 4. The polarity-inverted fixture (senseActiveHigh = false), fed a genuine
//    edge down to a steady LOW, decodes to the same logical OPEN - proving
//    polarity is fully parameterised, not assumed.
void test_polarity_inversion_steady_low_is_open_when_active_low(void) {
  StatusDecoder d;
  StatusDecoderConfig cfg = makeFixtureConfig(false); // senseActiveHigh = false
  d.begin(cfg, 0);

  uint32_t t = 0;
  d.tick(true, t);
  t += cfg.debounceMs;
  d.tick(true, t); // accepted edge -> debouncedLevel_ = true
  t += 1;
  d.tick(false, t);
  t += cfg.debounceMs;
  d.tick(false, t); // accepted edge -> debouncedLevel_ = false
  t += cfg.steadyConfirmMs;
  bool changed = d.tick(false, t);

  TEST_ASSERT_TRUE(changed);
  TEST_ASSERT_EQUAL(GateState::OPEN, d.currentState());
}

// 5. Steady since boot, with no edge ever detected, must still resolve to
//    CLOSED once steadyConfirmMs passes - a gate that's already closed at
//    power-on has no edge to wait for.
void test_steady_since_boot_with_no_edge_is_still_classified(void) {
  StatusDecoder d;
  StatusDecoderConfig cfg = makeFixtureConfig(true);
  d.begin(cfg, 0);

  bool changed = d.tick(false, cfg.steadyConfirmMs);

  TEST_ASSERT_TRUE(changed);
  TEST_ASSERT_EQUAL(GateState::CLOSED, d.currentState());
}

// 6. A measured period inside the slow-flash band decodes to OPENING.
void test_period_in_slow_band_is_opening(void) {
  StatusDecoder d;
  StatusDecoderConfig cfg = makeFixtureConfig(true);
  d.begin(cfg, 0);

  uint32_t midSlow = (cfg.slowFlashMinMs + cfg.slowFlashMaxMs) / 2;
  GateState s = feedOnePeriod(d, midSlow, cfg.debounceMs, 0);

  TEST_ASSERT_EQUAL(GateState::OPENING, s);
}

// 7. A measured period inside the fast-flash band decodes to CLOSING.
void test_period_in_fast_band_is_closing(void) {
  StatusDecoder d;
  StatusDecoderConfig cfg = makeFixtureConfig(true);
  d.begin(cfg, 0);

  uint32_t midFast = (cfg.fastFlashMinMs + cfg.fastFlashMaxMs) / 2;
  GateState s = feedOnePeriod(d, midFast, cfg.debounceMs, 0);

  TEST_ASSERT_EQUAL(GateState::CLOSING, s);
}

// 8. A measured period matching one of the five numbered fault bands
//    (1-5 flashes/sec [manual p.45]) decodes to FAULT. Using n=3 (~333ms).
void test_period_matching_fault_band_is_fault(void) {
  StatusDecoder d;
  StatusDecoderConfig cfg = makeFixtureConfig(true);
  d.begin(cfg, 0);

  uint32_t faultPeriodN3 = 1000UL / 3UL; // 333ms, within tolerance of band n=3
  GateState s = feedOnePeriod(d, faultPeriodN3, cfg.debounceMs, 0);

  TEST_ASSERT_EQUAL(GateState::FAULT, s);
}

// 9. A measured period matching no configured band leaves the state
//    unchanged rather than guessing.
void test_period_matching_no_band_leaves_state_unchanged(void) {
  StatusDecoder d;
  StatusDecoderConfig cfg = makeFixtureConfig(true);
  d.begin(cfg, 0);

  GateState before = d.currentState(); // UNKNOWN
  // 150ms: above fastFlashMax(100), below slowFlashMin(200), and not within
  // 5ms of any of the five fault periods (1000, 500, 333, 250, 200).
  GateState after = feedOnePeriod(d, 150, cfg.debounceMs, 0);

  TEST_ASSERT_EQUAL(before, after);
}

// 10. The same measured period (200ms) decodes differently depending on
//     the config's thresholds - proof the bands are genuinely data-driven,
//     not hardcoded. Under fixture A, 200ms falls in the slow (opening)
//     band; under fixture B (thresholds doubled and shifted), 200ms falls
//     in the fast (closing) band instead.
void test_thresholds_are_data_driven_not_hardcoded(void) {
  StatusDecoderConfig cfgA = makeFixtureConfig(true); // slow: 200-300

  StatusDecoderConfig cfgB = cfgA;
  cfgB.slowFlashMinMs = 400;
  cfgB.slowFlashMaxMs = 600;
  cfgB.fastFlashMinMs = 100;
  cfgB.fastFlashMaxMs = 200; // includes 200
  cfgB.faultFlashToleranceMs = 10;

  StatusDecoder dA;
  dA.begin(cfgA, 0);
  GateState sA = feedOnePeriod(dA, 200, cfgA.debounceMs, 0);

  StatusDecoder dB;
  dB.begin(cfgB, 0);
  GateState sB = feedOnePeriod(dB, 200, cfgB.debounceMs, 0);

  TEST_ASSERT_EQUAL(GateState::OPENING, sA);
  TEST_ASSERT_EQUAL(GateState::CLOSING, sB);
}

// 11. The changed flag fires exactly once on the tick where classification
//     flips, and false on a subsequent tick with the same classification.
void test_change_flag_fires_once_per_transition(void) {
  StatusDecoder d;
  StatusDecoderConfig cfg = makeFixtureConfig(true);
  d.begin(cfg, 0);

  uint32_t period = (cfg.slowFlashMinMs + cfg.slowFlashMaxMs) / 2;
  uint32_t t = 0;
  bool level = false;

  level = !level;
  d.tick(level, t);
  t += cfg.debounceMs;
  d.tick(level, t); // first accepted edge at t = debounceMs, no period yet

  // Second flip timed so the second edge is accepted exactly `period`
  // after the first (see feedOnePeriod's comment for why it's period -
  // debounceMs, not period, added here).
  t += (period - cfg.debounceMs);
  level = !level;
  d.tick(level, t);
  t += cfg.debounceMs;
  bool changedFirst = d.tick(level, t); // second accepted edge -> OPENING

  TEST_ASSERT_TRUE(changedFirst);
  TEST_ASSERT_EQUAL(GateState::OPENING, d.currentState());

  bool changedAgain = d.tick(level, t + 1); // settle-read, no new edge
  TEST_ASSERT_FALSE(changedAgain);
  TEST_ASSERT_EQUAL(GateState::OPENING, d.currentState());
}

// 12. millis() rollover across a period measurement: the period between
//     two accepted edges must compute correctly even when nowMs has
//     wrapped past 0xFFFFFFFF in between them.
void test_millis_rollover_across_period_measurement(void) {
  StatusDecoder d;
  StatusDecoderConfig cfg = makeFixtureConfig(true);
  const uint32_t kNearWrap = 0xFFFFFFFFu - 100u;
  d.begin(cfg, kNearWrap);

  uint32_t midSlow = (cfg.slowFlashMinMs + cfg.slowFlashMaxMs) / 2; // 250ms
  GateState s = feedOnePeriod(d, midSlow, cfg.debounceMs, kNearWrap);

  TEST_ASSERT_EQUAL(GateState::OPENING, s);
}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  UNITY_BEGIN();
  RUN_TEST(test_starts_unknown_not_closed);
  RUN_TEST(test_subdebounce_flicker_ignored);
  RUN_TEST(test_steady_high_is_open_when_active_high);
  RUN_TEST(test_polarity_inversion_steady_low_is_open_when_active_low);
  RUN_TEST(test_steady_since_boot_with_no_edge_is_still_classified);
  RUN_TEST(test_period_in_slow_band_is_opening);
  RUN_TEST(test_period_in_fast_band_is_closing);
  RUN_TEST(test_period_matching_fault_band_is_fault);
  RUN_TEST(test_period_matching_no_band_leaves_state_unchanged);
  RUN_TEST(test_thresholds_are_data_driven_not_hardcoded);
  RUN_TEST(test_change_flag_fires_once_per_transition);
  RUN_TEST(test_millis_rollover_across_period_measurement);
  return UNITY_END();
}
