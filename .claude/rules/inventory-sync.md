# Inventory: Keep inventory.yaml in Sync

Whenever parts are discussed in a session, bring `.claude/inventory.yaml` up to
date before the end of that turn. Do not wait to be asked.

This rule covers **when** to update. The transformation itself is defined once
in the `/inventory-update` skill; follow that rather than restating it here, so
the two cannot drift apart.

## Triggers

Update the inventory when the user mentions any of the following:

- A part has arrived or been received
- A part is now installed or in use
- A new part has been ordered
- A part is being deferred to a later phase
- A part is being ruled out, along with the reason
- A quantity, source, variant or note needs correcting

## Current Sections

`inventory.yaml` currently uses three sections:

- `already_have` - in hand, including parts already installed
- `to_buy` - still needed
- `optional` - contingency parts, bought only if a fallback is required

Two more are worth creating **when first needed**, rather than pre-emptively:

- `in_transit` - ordered but not arrived. Worth splitting out of `to_buy` once
  an order is placed, because "not ordered yet" and "arriving Thursday" block
  Phase 3 wiring very differently
- `rejected` - parts evaluated and ruled out, each with its reason. Several
  rejections already exist as inline asides (30AWG wire as too thin, CH340C in
  favour of CP2102, solder-splice heat shrink as incompatible with the
  solder-free build). Promoting them to their own section stops the same wrong
  part being reconsidered in six months

## Rules

- Never delete an entry. Rejected parts stay recorded with their reason: the
  reasoning is the value, not the row
- Preserve any caveat still to be acted on, such as the dry-layout check on the
  mounting box before drilling
- Never guess which entry a shorthand name refers to. Near-duplicates exist
  (the ferrule crimper tool versus the ferrule crimp terminals), so ask

## After Updating

Say what changed in one line, for example: "Inventory: optocoupler module moved
to already_have; to_buy is now just the hookup wire."

If `to_buy` empties, that closes Phase 1 procurement. Update `## Current Status`
in `CLAUDE.md` to match, per `.claude/rules/phase-gate.md`.
