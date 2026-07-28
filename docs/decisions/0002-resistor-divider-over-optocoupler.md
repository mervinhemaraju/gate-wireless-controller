# 0002 - Resistor Divider over Optocoupler for the Gate-Status Sense Line

- **Date:** 2026-07-28
- **Phase:** 2 (decision now; resistor values are Phase 3, see Consequences)
- **Status:** verified

## Context

The gate-status sense path reads the CP80 `LED` terminal (~4.5V DC, 20mA max
[manual p.31]) into an ESP32 GPIO (3.3V logic, not 5V tolerant). The inventory
already held a 2-channel PC817 optocoupler module for this, bought in Phase 1
against the general idea of using an optocoupler without a specific model
picked for its electrical suitability.

Bench-tested 2026-07-28 (full test in
[docs/wiring/01-optocoupler-front-end.md](../wiring/01-optocoupler-front-end.md)):
the module's onboard series resistor is 3k ohm on both the input and output
side. Driven from an ESP32 at both 3.3V and 5V, with the output pulled up
through its own onboard resistor, the sensed output never swung at all -
flat 3.27V regardless of input state. Fails to switch.

## Options Considered

**Buy a different optocoupler module or a bare PC817.** Keeps galvanic
isolation in principle. Requires another purchase and another round of this
same characterisation, with no guarantee the next module's resistor is
better sized without buying specifically to spec.

**Bare PC817 + a self-chosen resistor (this build stays solder-free, so this
would need a socket or a breakout with an open resistor position).** Solves
the resistor-sizing problem directly but needs a part not currently in
inventory and is more fiddly to source solder-free.

**Plain two-resistor voltage divider straight into the ESP32 GPIO (chosen).**
No switching-current threshold to fail against - a divider's behaviour is
Ohm's law, not a phototransistor's CTR curve. Needs two resistors, which were
already going to be bought locally for the optocoupler's series resistor
regardless of outcome.

## Decision

Drop the optocoupler. Use a plain resistor divider to bring the CP80 `LED`
terminal down to a safe ESP32 GPIO level.

The deciding factor: **this build gets no real galvanic isolation from the
optocoupler in the first place.** The ESP32 is powered from the CP80's own
12V aux rail through the buck converter, so ESP32 GND is already common with
CP80 COM. The optocoupler's only real contribution here was level-shifting
and some transient protection, both of which a divider also provides. There
is no isolation benefit being traded away.

## Consequences

- The PC817 module bought in Phase 1 is not used for gate-status sensing. It
  is moved to `rejected` in `.claude/inventory.yaml`, not deleted, so the
  reasoning stays on record and the part is not reconsidered later
- **The two resistor values are explicitly deferred to Phase 3, not decided
  here.** A divider's output voltage depends on the CP80 `LED` terminal's
  output impedance under load, which is not yet known: "approx 4.5V DC,
  20mA" [manual p.31] is consistent with either a stiff voltage source or a
  current-limited LED-driver output, and those need different divider maths.
  Per `.claude/rules/phase-gate.md`, this is exactly the class of fact that
  can only be pinned down by measuring the real terminal, in both gate
  states, before the divider is sized
- No firmware change: the GPIO still just reads a plain digital/analog pin.
  Nothing about Stage A.2 (GPIO pin assignments) or the firmware skeleton is
  affected by this pivot
- The general-purpose resistor assortment already planned as a purchase (for
  what was going to be the optocoupler's series resistor) covers the divider
  too, so this decision does not create a new purchase
- Losing transient protection that a PC817 would have added is a real,
  accepted trade-off. If the sense line proves noisy or a transient damages
  the GPIO in Phase 3, revisit with a clamping diode or a different
  optocoupler bought to spec, rather than reintroducing this module
