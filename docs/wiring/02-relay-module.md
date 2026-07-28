# Relay Module - Bench Test (Trigger Polarity)

- **Date:** 2026-07-28
- **Phase:** 2 (bench characterisation, no gate involved)
- **Status:** verified (trigger polarity confirmed by test; GPIO pin number
  not yet assigned, see Open Items)

## Module

Single-channel opto-isolated relay board, silkscreen "HW-803", "1 Relay
Module high/low level trigger" (Chinese: "1-channel optocoupler-isolated
relay module, supports high/low level trigger").

**Correction to earlier inventory note:** this board is single-channel, not
the 2-channel part the inventory previously described. No spare channel is
available on this board for the Phase 5 pedestrian-mode input; that will
need its own relay. `.claude/inventory.yaml` updated 2026-07-28.

Terminals:

```
NO  COM  NC        (contact output, front block)
DC+ DC-  IN        (coil power + logic input, rear block)
```

Plus a 2-pin H/L jumper selecting high- or low-level trigger on `IN`
[visual, 2026-07-28] - see Result below for which position is currently
fitted, confirmed by test rather than by reading the silkscreen position.

Relay itself: `JQC3F-05VDC-C`, rated 10A 250VAC / 10A 30VDC, 10A 125VAC /
10A 28VDC [marking on the relay can, 2026-07-28]. Coil is 5V DC. Well within
rating for the CP80 `TRG`-`COM` dry contact, which needs only a momentary
low-current closure [manual p.31, p.41].

## Functional test

Coil powered from an ESP32 dev board's own `5V`/`VIN` rail (USB power only,
no firmware running):

- `DC+` <- ESP32 `5V`/`VIN`
- `DC-` <- ESP32 `GND`
- `IN` tried in three states, one at a time
- Multimeter: continuity/buzzer mode, probes on `NO` and `COM`

| `IN` state | Relay LED / click | `NO`-`COM` |
|---|---|---|
| Floating (disconnected) | Off | Open (no beep, meter reads `1`) |
| Driven LOW (tied to ESP32 `GND`) | Off | Open (no beep, meter reads `1`) |
| Driven HIGH (tied to ESP32 `3V3`) | On (LED lit, audible click) | Closed (beep, meter reads ~0.01 ohm) |

[measured 2026-07-28]

## Result

**Active-HIGH, in the board's current jumper position.** Driving `IN` high
(3.3V from the ESP32 was sufficient - no need for 5V logic) energizes the
relay. Floating and driven-low behave identically (relay off), so no pull
resistor surprise either way while the jumper stays in this position.

**Safety-relevant finding:** floating `IN` reads as "off", the same as
driven-low. An ESP32 GPIO defaults to high-impedance at boot/reset before
firmware configures it, so with this board in its current jumper position,
the gate does not trigger itself during a boot or reset. This is the
behaviour `.claude/CLAUDE.md` calls out as the failure mode to avoid.
Firmware should still explicitly drive the trigger GPIO low at startup
rather than relying on this floating behaviour alone, since it is cheap
insurance and the jumper could be moved later.

## Consequence for firmware

The GPIO driving `IN` must be driven **HIGH** to trigger, and held **LOW**
at idle. This is the opposite polarity from a common assumption ("active-low
relay boards are common" per `.claude/rules/wiring-research.md`) - this
specific board, in its current jumper position, is not one of those. Do not
assume active-low without re-reading this document.

## Open Items

- GPIO pin number not yet assigned - waits on Stage A.2's remaining item
  (dry-layout check) and the overall pin map, not on this result
- The H/L jumper could be moved to change this polarity later. If it is ever
  physically moved, this document's result is stale until re-tested
