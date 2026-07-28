# 0003 - Leave the Mounting Box Open Rather Than Sealed

- **Date:** 2026-07-29
- **Phase:** 2/3 boundary (decided during the bench dry-layout check, applies
  once physical mounting happens in Phase 3)
- **Status:** verified

## Context

The IP67 ABS enclosure in the inventory was bought in Phase 1 to weatherproof
the ESP32, breakout board, relay, and buck converter inside the D3 motor
housing. The Stage A dry-layout check (2026-07-29, photo in
[docs/wiring/03-mounting-layout.md](../wiring/03-mounting-layout.md)) confirmed
everything fits on the box floor with room to spare.

During that check, the user decided the box will stay open (no lid fitted)
once installed, rather than sealed as originally implied by buying an IP67
part.

## Options Considered

**Sealed IP67 box (original plan).** Protects the electronics even if the D3
motor housing's own seal is compromised or the housing is opened for
servicing. Requires cable glands or grommets at every wire entry point to
actually preserve the IP67 rating - none are currently in the inventory, so
this would have been a new purchase. A drilled, unglanded entry hole would
have made the "IP67" rating nominal rather than real regardless.

**Open box, D3 housing as the only barrier (chosen).** No cable glands
needed - wires simply route in over the open top. Servicing later (reflashing
over USB, rewiring) never requires unsealing anything. Relies entirely on the
D3 motor housing's own rating (IP55 [manual p.10]) remaining intact.

## Decision

Leave the mounting box open inside the motor housing.

The deciding factor: the D3 housing is already the real environmental
barrier (IP55, a sealed unit by design), so a second sealed layer inside it
was redundant, and the redundancy had a real cost (cable glands, a fiddlier
build, harder future servicing).

## Consequences

- The IP67 box no longer needs cable glands or grommets. Removes a gap that
  had been flagged but not yet added to `to_buy`
- Environmental protection for the electronics now depends entirely on the
  D3 motor housing staying closed and its seal remaining intact. If the
  housing is left open for extended periods (servicing, an ill-fitting
  cover) or its seal has degraded with age, the electronics have no second
  line of defence
- The box's role changes from "sealed enclosure" to "mounting tray" -
  components still benefit from being organised and screwed down inside it,
  just not from being sealed
- First real-world check of whether this was the right call is Phase 3
  install day, when the actual housing condition can be inspected. If the D3
  housing turns out to admit dust or moisture, revisit this decision before
  finishing the install
