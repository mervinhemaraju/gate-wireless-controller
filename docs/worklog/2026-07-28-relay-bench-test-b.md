# Worklog - 2026-07-28 (b)

- **Date:** 2026-07-28
- **Phase:** 2 (bench work)

Second session today. Continues from
[2026-07-28 - Optocoupler Bench Test](2026-07-28-optocoupler-bench-test.md),
which closed out with "Stage A.2: bench-check the relay module's polarity"
as the next item. That item is this session's work.

## Done

- Bench-tested the relay module (Stage A.2), same method as Stage A.1: an
  ESP32 dev board on USB power only (no firmware) and a multimeter, entirely
  a desk task.
  - Corrected a standing inventory error: the module is a **single-channel**
    opto-isolated relay board (silkscreen "HW-803", "1 Relay Module
    high/low level trigger"), not the 2-channel part the inventory had
    described since Phase 1. There is no spare channel on this board for
    the Phase 5 pedestrian-mode input; that will need its own relay,
    bought when Phase 5 actually starts.
  - Confirmed the relay coil is `JQC3F-05VDC-C`, 5V DC rated - lines up
    with the existing 12V-to-5V buck converter already planned to power the
    ESP32, so the same 5V rail can power this relay's coil too.
  - Ran the functional test: coil powered from the ESP32's `5V`/`VIN` rail,
    `IN` tried floating, driven low, and driven high, contact state read on
    `NO`-`COM` with the multimeter in continuity mode.
  - Result: **active-HIGH**, in the board's current jumper position.
    Floating and driven-low both read as "off", same as each other. Driven
    high (3.3V from the ESP32 was enough) energized the relay - audible
    click, LED lit, `NO`-`COM` continuity closed.
  - Full record in `docs/wiring/02-relay-module.md`.
- Updated `.claude/inventory.yaml`: relay entry corrected from "2-channel"
  to "single-channel, HW-803", with the coil voltage and trigger-polarity
  finding folded in.
- Indexed the new wiring document in `docs/README.md`.

## Learned

- Floating `IN` behaves the same as driven-low on this board (both "off").
  That means an ESP32 GPIO's default high-impedance state at boot/reset
  does not accidentally trigger the gate, with the jumper in its current
  position. This is the specific failure mode `.claude/CLAUDE.md` warns
  about, and it is not a risk here as currently configured - though firmware
  should still explicitly drive the pin low at startup rather than depend
  on this passively, since the jumper could be moved later.
- The inventory's "2-channel" note for this relay had gone unchecked since
  Phase 1. Second instance today (after the optocoupler's unverified
  resistor) of a part description in the inventory not matching the
  physical part - worth treating every inventory note describing an
  electrical property as provisional until bench-confirmed, not just the
  ones already flagged as uncertain.

## Did Not Work

Nothing failed this session. Both the coil-power assumption (5V is enough,
sourced from the ESP32's own 5V rail) and the trigger test came back clean
on the first attempt.

## Measurements

- Relay trigger polarity: active-HIGH in the board's current jumper
  position [measured 2026-07-28]. Full table (floating / low / high, each
  against `NO`-`COM` continuity) in `docs/wiring/02-relay-module.md`.

## Next

- Mounting-box dry-layout check (still open from earlier notes, still
  indoor).
- GPIO pin assignments: now unblocked on the trigger side (polarity known),
  still waiting on the dry-layout check before numbers are picked.
- Stage B (software): firmware skeleton can now assume active-HIGH trigger
  logic with a LOW idle default.
- Same loose end as last time, still unaddressed: bench photos live in
  `images/` rather than `docs/wiring/img/`, and neither wiring document
  embeds or links them.
- Phase 5 note (not to act on yet): pedestrian-mode input will need its own
  relay module, since this one has no spare channel.
