# LED Indicator Lights / STATUS Flash Codes (D3/D5 Manual)

- **Date:** 2026-07-22
- **Phase:** 3 reference (feeds the Phase 2 decoder state machine and the
  Phase 3 flash-code-to-state confirmation)
- **Status:** verified (faithful transcription of manual p.45, Table 3). The
  mapping is the manufacturer's documented behaviour; it still has to be
  confirmed against this gate's actual STATUS output before the decoder table
  is finalised (Phase 3).

Source: Centurion D3/D5 installation manual p.45 "LED Indicator Lights", read
visually from the scanned PDF (poppler render, 2026-07-22).

## STATUS LED, during NORMAL operation (Red) [manual p.45]

Steady states:

| STATUS output | Meaning |
|---|---|
| Off (steady) | Gate is closed (fully closed) |
| On (steady) | Gate is open (fully open) |
| Slow continuous flash | Gate is opening |
| Fast continuous flash | Gate is closing |

Numbered flash codes (faults / conditions):

| STATUS output | Meaning |
|---|---|
| 1 flash/sec | Pillar light on permanently |
| 2 flashes/sec | No mains |
| 3 flashes/sec | Battery low detection |
| 4 flashes/sec | Collision detector operated |
| 5 flashes/sec | Microprocessor reset |

## STATUS LED, during PROGRAMME mode (Red)

The number of flashes corresponds to the STATUS or COUNT value being set (see
the Controller Features table, manual p.44).

## Other indicator LEDs [manual p.45]

| Name | Colour | On | Off |
|---|---|---|---|
| TRG | Red | Signal present | No signal |
| IRB | Green | Safeties clear | Safeties obstructed |
| FRX | Red | Signal present | No signal |
| PED | Red | Signal present | No signal |
| LCK | Green | System ready to operate | System locked |
| SET | Red | Signal present | No signal |
| CHARGER ON | Green | Battery charging | No charge voltage present |

- **L1** (normal operation, Red): origin diagnostic. Off when gate on the
  "CLOSED" side, On when gate on the "OPEN" side.
- **L2** (normal operation, Red): DOSS diagnostic (changes state to indicate
  DOSS pulses).
- In programme mode, L1 + L2 both on = system reset to factory default.

## Decoder note (why this is provisional for the firmware)

The manual does not give exact frequencies for "slow continuous flash"
(opening) or "fast continuous flash" (closing), and those have to be told apart
from the numbered flash/sec codes (e.g. 1 flash/sec = pillar light, 2/sec = no
mains). The decoder therefore needs the real gate's flash timing measured
before the flash-code-to-state table is fixed. This is the Phase 3
characterisation TODO called out in CLAUDE.md. [UNVERIFIED on this gate:
measure flash periods for opening / closing / each fault code]
