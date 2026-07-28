# Worklog - 2026-07-28

- **Date:** 2026-07-28
- **Phase:** 1 (closed out) / 2 (bench work)

First-ever worklog entry, so this covers the whole day, not just the delta
since a previous one.

## Done

- Hookup wire (6-colour, UL1007, 22AWG) arrived. Moved from `in_transit` to
  `already_have`; `to_buy` and `in_transit` both emptied, closing Phase 1
  procurement (committed separately, commit `ca30abf`).
- Bench-tested the PC817 2-channel optocoupler module (Stage A.1 of the
  working plan), using only an ESP32 dev board (USB power only, no firmware)
  and a multimeter. Entirely a desk task, no gate involved:
  - Confirmed topology: simple two-terminal per channel. `IN1`/`G` input,
    `V1`/`G` output (channel 1); `IN2`/`G`, `V2`/`G` (channel 2, unused).
  - Read the onboard SMD resistor markings: `302` = 3k ohm, on all four
    resistor positions (input and output side, both channels).
  - Ran the functional test at both 3.3V and 5V ESP32 drive: the output held
    a flat 3.27V in every combination tried (input connected/disconnected,
    3.3V/5V drive). No switching at all.
  - Verdict: **fail**. Full record in
    `docs/wiring/01-optocoupler-front-end.md`.
- Decided to drop the optocoupler for the gate-status sense line and replace
  it with a plain resistor divider. Recorded in
  `docs/decisions/0002-resistor-divider-over-optocoupler.md`.
- Updated `.claude/inventory.yaml`: optocoupler module moved to a new
  `rejected` section with the reason; added a general resistor assortment to
  `to_buy` (specific values deferred to Phase 3).
- Updated `.claude/CLAUDE.md` `## Current Status`: Phase 1 reopened for the
  one resistor-assortment item, not blocking anything.
- Indexed both new documents in `docs/README.md`.
- User has 5 spare through-hole resistors already on hand. Colour bands read
  (uncertain at photo resolution) as roughly 40-400 megaohm - too high and
  too narrow a single value to be useful for the eventual divider. Still
  recommended the local-shop assorted pack rather than relying on these.

## Learned

- The optocoupler bought in Phase 1 was never matched to the CP80's
  low-current (~4.5V DC, 20mA [manual p.31]) LED status output. Its onboard
  3k resistor is too high for that source - the inventory note had already
  flagged this risk before today, but it had never actually been resolved
  until this bench test.
- The sourced decision table in the working plan (3k -> "marginal, then
  fail") predicted the real result accurately at both 3.3V and 5V drive.
  Worth trusting that kind of table again for the next part choice.
- The optocoupler was never providing real galvanic isolation in this
  design anyway, since the ESP32 is powered from the CP80's own 12V aux rail
  through the buck converter (shared ground). That made dropping it for a
  plain divider a low-cost decision, not a compromise.

## Did Not Work

- Trying to isolate and measure the onboard SMD resistor directly with
  multimeter probes. The pads are too small to probe reliably; reading the
  printed marking (`302`) directly off the part was the practical path.
- Continuity/diode-test (buzzer) mode was not useful for finding the
  resistor value - 3k is well above the typical beep threshold. Needed a
  plain resistance-range measurement instead, and even that could not
  cleanly target the resistor's own pads.

## Measurements

- PC817 module onboard resistor: `302` marking = 3k ohm, both channels,
  input and output side [visual, 2026-07-28].
- PC817 module output voltage under functional test: flat 3.27V at both
  3.3V and 5V input drive, input connected and disconnected [measured
  2026-07-28].

Both fully recorded, with setup and conditions, in
`docs/wiring/01-optocoupler-front-end.md`.

## Next

- Stage A.2: bench-check the relay module's polarity. Still indoor, no gate
  trip needed.
- Buy the resistor assortment from the local shop (already in `to_buy`).
  Exact divider values still wait for Phase 3 - the CP80 `LED` terminal has
  to be measured under load first (see ADR 0002).
- Loose end, low priority: the bench photos taken today
  (`images/optocoupler/`, `images/tools/`) live outside
  `docs/wiring/img/`, and `01-optocoupler-front-end.md` does not embed or
  link any of them, both of which `docs-conventions.md` calls for. Worth
  moving the relevant ones into `docs/wiring/img/` and linking them next
  time this document is touched.
- Nothing physical was left half-done. No mains or battery work happened
  this session - everything was desk/bench only.
