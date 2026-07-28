# PC817 Optocoupler Front End - Bench Test

- **Date:** 2026-07-28
- **Phase:** 2 (bench characterisation, no gate involved)
- **Status:** superseded by [0002](../decisions/0002-resistor-divider-over-optocoupler.md).
  The module tested here is rejected for the gate-status sense line. This
  document records the measurement and the reasoning, not a working design.

## Module

2-channel PC817 breakout, silkscreen "817 Module" / "2 Channel Isolation".
Terminals, both channels symmetric:

```
IN1  G          IN2  G      (input side, bottom edge of the board)
 G  V1           G  V2      (output side, top edge of the board)
```

Topology confirmed by inspection: simple two-terminal per channel on both
input and output. No shared VCC/GND rail, no series indicator LED in the
input path [visual inspection, 2026-07-28].

## Onboard resistor

SMD marking `302` on all four resistor positions (input and output side,
both channels) = 3k ohm (EIA 3-digit code: `30` x 10^2) [marking read
directly off the part, 2026-07-28].

An in-circuit ohmmeter check across the input terminals was attempted first
and abandoned: a DMM in resistance mode across `IN1`-`G` reads through the
series resistor **and** the PC817's internal LED junction, and since the
onboard resistors are SMD (their own two solder pads are too small to
reliably probe directly), no clean isolated reading of the resistor alone
was obtained this way. The printed marking is the value of record here,
consistent across four separate instances of the same part on one board.

## Functional test

Input driven from an ESP32 dev board, powered over USB (no firmware
running - just the board's own regulated rails):

- Input: ESP32 `3V3` -> `IN1`, ESP32 `GND` -> `G` (input side)
- Output: ESP32 `3V3` -> `V1` (through the module's own onboard pull-up
  resistor), ESP32 `GND` -> `G` (output side)
- Multimeter: DC volts, probes on `V1` (red) and `G` (black), output side

| Input drive | Input connected? | `V1` reading |
|---|---|---|
| ESP32 3.3V rail | Yes | 3.27V |
| ESP32 3.3V rail | No (3V3 wire lifted from IN1) | 3.27V |
| ESP32 5V/VIN rail | Yes | 3.27V |
| ESP32 5V/VIN rail | No | 3.27V |

[measured 2026-07-28]

No swing at all, at either drive voltage. Per the decision table in
`/Users/jeeyah/.claude/plans/quiet-cooking-sutherland.md` (Stage A.1), a 3k
input resistor predicts 0.7mA at 3.3V and 1.1mA at the CP80's 4.5V, both in
the "marginal, then fail" band even before accounting for PC817 CTR being
uncharacterised below 1mA [PC817 datasheet]. The flat result is consistent
with that prediction, not with a wiring mistake: both drive voltages and
both input states were tried and gave the same reading.

## Verdict

**Fail.** This module does not switch reliably from a source in the
CP80 `LED` terminal's voltage class (~4.5V DC, 20mA max [manual p.31]).

## Consequence

Decision recorded in [0002](../decisions/0002-resistor-divider-over-optocoupler.md):
drop the optocoupler from the gate-status sense line, use a plain resistor
divider instead. See that document for the reasoning and what is deferred to
Phase 3.
