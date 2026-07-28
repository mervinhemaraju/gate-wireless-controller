// Native, hardware-free tests for gate_control - the safety-critical
// trigger/cooldown/dedup state machine. Run with: pio test -e native
//
// The relay is verified via a spy PinWriteFn rather than real hardware.

#include <unity.h>

#include <cstdio>
#include <cstdint>

#include "gate_control.h"

namespace {

int g_writeCount = 0;
int g_lastLevel = -1;

void spyWriteRelay(int level) {
  g_writeCount++;
  g_lastLevel = level;
}

void resetSpy() {
  g_writeCount = 0;
  g_lastLevel = -1;
}

// tick() only evaluates the CURRENT state's own transition condition per
// call (by design - it mirrors one real loop() iteration, not a fast
// forward). Jumping nowMs straight to a combined "pulse + cooldown" time
// and calling tick() once would leave the controller stuck in COOLDOWN,
// since cooldownStartMs_ gets set to that same far-future nowMs. Real
// firmware never hits this because loop() ticks constantly; tests must
// tick() at each boundary explicitly to reach IDLE, same as two real loop
// iterations would.
void advanceThroughPulseAndCooldown(GateControl& gc, uint32_t& t) {
  t += TRIGGER_PULSE_MS;
  gc.tick(t);
  t += TRIGGER_COOLDOWN_MS;
  gc.tick(t);
}

} // namespace

void setUp(void) {
  resetSpy();
}

void tearDown(void) {}

// 1. begin() forces the relay to its idle level immediately.
void test_begin_forces_idle(void) {
  GateControl gc;
  gc.begin(spyWriteRelay, 1000);
  TEST_ASSERT_EQUAL_INT(1, g_writeCount);
  TEST_ASSERT_EQUAL_INT(RELAY_IDLE_LEVEL, g_lastLevel);
}

// 2. A fresh request_id on an idle controller is accepted and drives the
//    relay active immediately.
void test_fresh_request_accepted_and_drives_relay(void) {
  GateControl gc;
  gc.begin(spyWriteRelay, 1000);
  resetSpy();

  GateControlResult r = gc.requestTrigger("req-1", 1000);

  TEST_ASSERT_EQUAL(GateControlResult::ACCEPTED, r);
  TEST_ASSERT_EQUAL_INT(1, g_writeCount);
  TEST_ASSERT_EQUAL_INT(RELAY_ACTIVE_LEVEL, g_lastLevel);
}

// 3. Mid-pulse, a different request_id is rejected as busy and never
//    touches the relay.
void test_midpulse_rerequest_rejected_busy(void) {
  GateControl gc;
  gc.begin(spyWriteRelay, 1000);
  gc.requestTrigger("req-1", 1000);
  resetSpy();

  gc.tick(1000 + TRIGGER_PULSE_MS / 2);
  GateControlResult r = gc.requestTrigger("req-2", 1000 + TRIGGER_PULSE_MS / 2);

  TEST_ASSERT_EQUAL(GateControlResult::REJECTED_BUSY, r);
  TEST_ASSERT_EQUAL_INT(0, g_writeCount);
}

// 4. The relay stays active right up to the pulse-end boundary, and goes
//    idle exactly at TRIGGER_PULSE_MS, not before and not late.
void test_pulse_end_boundary(void) {
  GateControl gc;
  gc.begin(spyWriteRelay, 0);
  gc.requestTrigger("req-1", 0);
  resetSpy();

  gc.tick(TRIGGER_PULSE_MS - 1);
  TEST_ASSERT_EQUAL_INT(0, g_writeCount); // not yet

  gc.tick(TRIGGER_PULSE_MS);
  TEST_ASSERT_EQUAL_INT(1, g_writeCount);
  TEST_ASSERT_EQUAL_INT(RELAY_IDLE_LEVEL, g_lastLevel);
}

// 5. A request during the post-pulse cooldown window is rejected.
void test_cooldown_rejection(void) {
  GateControl gc;
  gc.begin(spyWriteRelay, 0);
  gc.requestTrigger("req-1", 0);
  gc.tick(TRIGGER_PULSE_MS); // pulse ends, cooldown starts

  GateControlResult r = gc.requestTrigger("req-2", TRIGGER_PULSE_MS + 1);

  TEST_ASSERT_EQUAL(GateControlResult::REJECTED_COOLDOWN, r);
}

// 6. Once cooldown fully elapses, a new request_id is accepted again.
void test_post_cooldown_acceptance(void) {
  GateControl gc;
  gc.begin(spyWriteRelay, 0);
  gc.requestTrigger("req-1", 0);
  gc.tick(TRIGGER_PULSE_MS);

  uint32_t cooldownEnd = TRIGGER_PULSE_MS + TRIGGER_COOLDOWN_MS;
  gc.tick(cooldownEnd);
  GateControlResult r = gc.requestTrigger("req-2", cooldownEnd);

  TEST_ASSERT_EQUAL(GateControlResult::ACCEPTED, r);
}

// 7. The exact same request_id resubmitted immediately (simulating QoS 1
//    redelivery) is a duplicate, even though the controller is idle and
//    would otherwise accept it.
void test_duplicate_rejected_immediately(void) {
  GateControl gc;
  gc.begin(spyWriteRelay, 0);
  gc.requestTrigger("req-1", 0);
  resetSpy();

  GateControlResult r = gc.requestTrigger("req-1", 1);

  TEST_ASSERT_EQUAL(GateControlResult::DUPLICATE, r);
  TEST_ASSERT_EQUAL_INT(0, g_writeCount);
}

// 8. The same request_id resubmitted well after cooldown has fully elapsed
//    is STILL a duplicate. This is the key property: dedup outlives
//    cooldown, because QoS 1 redelivery can arrive arbitrarily late.
void test_duplicate_rejected_after_cooldown_elapsed(void) {
  GateControl gc;
  gc.begin(spyWriteRelay, 0);
  gc.requestTrigger("req-1", 0);
  gc.tick(TRIGGER_PULSE_MS);

  uint32_t cooldownEnd = TRIGGER_PULSE_MS + TRIGGER_COOLDOWN_MS;
  gc.tick(cooldownEnd);

  GateControlResult r = gc.requestTrigger("req-1", cooldownEnd + 10000);

  TEST_ASSERT_EQUAL(GateControlResult::DUPLICATE, r);
}

// 9. Ring-buffer eviction is bounded and documented: after
//    REQUEST_ID_HISTORY_SIZE + 1 distinct ids, the very first one is no
//    longer flagged as a duplicate (it was evicted), while the most recent
//    one still is.
void test_ring_buffer_eviction(void) {
  GateControl gc;
  gc.begin(spyWriteRelay, 0);

  uint32_t t = 0;
  char ids[REQUEST_ID_HISTORY_SIZE + 1][16];

  for (size_t i = 0; i < REQUEST_ID_HISTORY_SIZE; ++i) {
    std::snprintf(ids[i], sizeof(ids[i]), "id-%zu", i);
    GateControlResult r = gc.requestTrigger(ids[i], t);
    TEST_ASSERT_EQUAL(GateControlResult::ACCEPTED, r);
    advanceThroughPulseAndCooldown(gc, t); // back to idle before the next
  }

  // One more distinct id: evicts ids[0] from the history.
  std::snprintf(ids[REQUEST_ID_HISTORY_SIZE], sizeof(ids[REQUEST_ID_HISTORY_SIZE]), "id-%zu",
                REQUEST_ID_HISTORY_SIZE);
  GateControlResult rNew = gc.requestTrigger(ids[REQUEST_ID_HISTORY_SIZE], t);
  TEST_ASSERT_EQUAL(GateControlResult::ACCEPTED, rNew);
  advanceThroughPulseAndCooldown(gc, t);

  // ids[0] was evicted - it's accepted again, not flagged duplicate.
  GateControlResult r0 = gc.requestTrigger(ids[0], t);
  TEST_ASSERT_EQUAL(GateControlResult::ACCEPTED, r0);
  advanceThroughPulseAndCooldown(gc, t);

  // The most recently added id is still remembered.
  GateControlResult rLast = gc.requestTrigger(ids[REQUEST_ID_HISTORY_SIZE], t);
  TEST_ASSERT_EQUAL(GateControlResult::DUPLICATE, rLast);
}

// 10. Across pulse -> cooldown -> idle -> pulse again, the relay is only
//     ever asserted active for exactly TRIGGER_PULSE_MS, never twice in a
//     row without an intervening ACCEPTED result.
void test_relay_active_duration_exact(void) {
  GateControl gc;
  gc.begin(spyWriteRelay, 0);
  resetSpy();

  gc.requestTrigger("req-1", 0);
  TEST_ASSERT_EQUAL_INT(RELAY_ACTIVE_LEVEL, g_lastLevel);

  gc.tick(TRIGGER_PULSE_MS - 1);
  TEST_ASSERT_EQUAL_INT(RELAY_ACTIVE_LEVEL, g_lastLevel); // still active, one ms early

  gc.tick(TRIGGER_PULSE_MS);
  TEST_ASSERT_EQUAL_INT(RELAY_IDLE_LEVEL, g_lastLevel); // idle exactly on time
}

// 11. millis() rollover: uint32_t wraps at 0xFFFFFFFF (~49.7 days on real
//     hardware). A pulse started just before the wrap must still end and
//     cool down correctly once nowMs has wrapped around past zero.
void test_millis_rollover_across_pulse_and_cooldown(void) {
  GateControl gc;
  const uint32_t kNearWrap = 0xFFFFFFFFu - 50u; // 50ms before wraparound
  gc.begin(spyWriteRelay, kNearWrap);
  resetSpy();

  GateControlResult r = gc.requestTrigger("req-1", kNearWrap);
  TEST_ASSERT_EQUAL(GateControlResult::ACCEPTED, r);

  // This addition wraps past 0xFFFFFFFF, exactly as it would on real
  // hardware after ~49.7 days of uptime.
  uint32_t afterPulse = kNearWrap + TRIGGER_PULSE_MS;
  gc.tick(afterPulse);
  TEST_ASSERT_EQUAL_INT(RELAY_IDLE_LEVEL, g_lastLevel);

  uint32_t cooldownEnd = afterPulse + TRIGGER_COOLDOWN_MS;
  gc.tick(cooldownEnd);
  GateControlResult r2 = gc.requestTrigger("req-2", cooldownEnd);
  TEST_ASSERT_EQUAL(GateControlResult::ACCEPTED, r2);
}

// 12. An oversized request_id is rejected as malformed, without touching
//     the relay or being added to the dedup history.
void test_oversized_request_id_rejected(void) {
  GateControl gc;
  gc.begin(spyWriteRelay, 0);
  resetSpy();

  char oversized[REQUEST_ID_MAX_LEN + 10];
  for (size_t i = 0; i < sizeof(oversized) - 1; ++i) {
    oversized[i] = 'a';
  }
  oversized[sizeof(oversized) - 1] = '\0';

  GateControlResult r = gc.requestTrigger(oversized, 0);
  TEST_ASSERT_EQUAL(GateControlResult::REJECTED_MALFORMED_ID, r);
  TEST_ASSERT_EQUAL_INT(0, g_writeCount);

  // Not recorded in history: a fresh, valid id is unaffected and the
  // controller is still idle.
  GateControlResult r2 = gc.requestTrigger("req-valid", 0);
  TEST_ASSERT_EQUAL(GateControlResult::ACCEPTED, r2);
}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  UNITY_BEGIN();
  RUN_TEST(test_begin_forces_idle);
  RUN_TEST(test_fresh_request_accepted_and_drives_relay);
  RUN_TEST(test_midpulse_rerequest_rejected_busy);
  RUN_TEST(test_pulse_end_boundary);
  RUN_TEST(test_cooldown_rejection);
  RUN_TEST(test_post_cooldown_acceptance);
  RUN_TEST(test_duplicate_rejected_immediately);
  RUN_TEST(test_duplicate_rejected_after_cooldown_elapsed);
  RUN_TEST(test_ring_buffer_eviction);
  RUN_TEST(test_relay_active_duration_exact);
  RUN_TEST(test_millis_rollover_across_pulse_and_cooldown);
  RUN_TEST(test_oversized_request_id_rejected);
  return UNITY_END();
}
