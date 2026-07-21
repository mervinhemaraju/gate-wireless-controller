# Wiring Research: Verify Before Proposing

Applies to **every** suggestion, plan, diagram or answer that touches physical
connections: CP80 terminals, the buck converter, the relay, the optocoupler,
power rails, grounds, or anything inside the motor housing.

The cost of being wrong here is not a failed test. It is a damaged CP80 board,
a dead ESP32, a gate that opens itself at 3am, or someone hurt by a moving
gate. **A confident guess is worse than an admitted gap**, because a guess gets
wired in and an admitted gap gets looked up.

Complements `/wiring-check`: this rule governs what is proposed, that skill
governs what happens before the work starts.

## Never Answer From Memory

General knowledge of gate motors, relay modules and optocouplers is not
knowledge of *this* CP80 board. Before proposing any connection:

1. **Check the D3/D5 manual first.** Use `/d3-manual <topic>`. It is the
   primary source for terminal names, terminal numbering, the status LED flash
   codes, and the aux supply rating
2. **Check the project's own record second.** `docs/` holds measured values and
   past decisions. A multimeter reading taken at this gate beats any datasheet
   figure
3. **Check component datasheets third**, at the exact part in the inventory:
   PC817, the specific buck module, the relay module. Confirm the part number
   against `inventory.yaml` rather than assuming a generic equivalent
4. **Only then** write the proposal

## Cite Every Claim

Each electrical claim in a plan carries its source inline:

- `[manual p.23]` for the D3/D5 manual, with the page number
- `[measured 2026-07-21]` for a multimeter reading recorded in `docs/`
- `[PC817 datasheet]` for a component figure
- `[UNVERIFIED]` for anything not yet confirmed

An uncited electrical claim is a bug. If a plan contains `[UNVERIFIED]` items,
say so at the top and list what has to be checked before any wiring starts.

## Conflicting or Missing Sources

- Sources disagree: present both, say which is more authoritative and why, and
  recommend a multimeter measurement to settle it. Do not average them or pick
  the convenient one
- The manual is silent: say so explicitly. Offer a measurement procedure to
  establish the answer safely, with power isolated
- A listing or forum post is the only source: treat it as unverified. Vendor
  listings for the AliExpress modules in this build are frequently wrong about
  polarity and pinout

## Measure Rather Than Assume

Prefer a measurement over a specification for anything this build depends on:

- The 12V aux rail is nominal. Measure it loaded and unloaded before sizing the
  buck converter input
- The status LED output voltage and its polarity must be measured in both
  states before choosing the PC817 series resistor
- Relay module boards are commonly active-low but not always. Measure, do not
  trust the silkscreen
- Confirm the common ground between the CP80 and the ESP32 side explicitly. A
  floating or wrongly shared ground is the most common way this class of build
  destroys a board

## Always State the Failure Mode

Every wiring proposal ends with what happens if it is wrong: which component
dies, whether it fails safe or fails with the gate open, and whether the error
is reversible. If a mistake would put mains voltage anywhere near the ESP32, or
would drive the gate unexpectedly, lead with that rather than burying it.

## Respect the Hard Conditions

Re-read the hard conditions in `CLAUDE.md` before proposing wiring. In
particular: nothing connects the Pi to the motor, everything stays on the 12V
side of the CP80, and the NOVA remote receiver is not touched.
