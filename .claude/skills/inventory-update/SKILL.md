---
name: inventory-update
description: Move arrived parts from to_buy to already_have in inventory.yaml, condensing their notes and updating the Phase 1 status. Invoke with /inventory-update [list of parts that arrived].
allowed-tools: Read(.claude/inventory.yaml), Read(.claude/CLAUDE.md), Edit(.claude/inventory.yaml), Edit(.claude/CLAUDE.md)
---

# Inventory Update: $ARGUMENTS

Parts listed in `$ARGUMENTS` have arrived. Move them from `to_buy` to
`already_have` in `.claude/inventory.yaml`.

If `$ARGUMENTS` is empty, ask which parts arrived. Never guess.

## 1. Read Before Matching

Read `.claude/inventory.yaml` in full. The user writes in shorthand
("wago connectors", "the breakout board"), so match loosely against the
`item` field, but confirm the match is unique.

## 2. Resolve Ambiguity by Asking

Some entries are near-duplicates that differ in an important way. The known
trap: **Ferrule crimper** (the tool) versus **Ferrule crimp terminals, E0508**
(the consumable). "Ferrule crimps" could mean either.

When a shorthand name plausibly matches more than one entry, or matches an
entry already sitting in `already_have`, stop and ask which one is meant.
State both candidates explicitly. Do not silently pick the likelier one.

## 3. Transform Each Entry

`to_buy` entries are pre-purchase decision records; `already_have` entries are
post-purchase facts. Reshape accordingly:

- Keep `item` and `quantity` unchanged
- Collapse `example` and `purpose` into a single `notes` field, keeping the
  identifying detail (part number, size, variant) and the reason it is in the
  build. Drop the shopping detail: AliExpress item numbers, "not the 30AWG
  default some listings pre-select", and similar buying-decision context that
  no longer applies once the part is on the bench
- Drop `approx_price_usd` entirely
- Preserve any caveat that still has to be acted on later, for example the
  dry-layout check on the mounting box before drilling

Place moved entries at the end of `already_have`, in the order the user
listed them.

## 4. Update Phase Status

Read the `## Current Status` section of `.claude/CLAUDE.md`:

- If `to_buy` is now empty, Phase 1 procurement is done. Update the status
  line to say so and note that Phase 3 wiring is unblocked once any in-transit
  parts arrive
- If items remain, update the status to name what is still outstanding
- Leave the WiFi verification notes alone unless the user mentions them

## 5. Propose, Then Apply

Per the global no-auto-changes rule: show which entries move, the condensed
`notes` for each, what remains in `to_buy`, and any status line change. Wait
for approval before editing.

Do not commit or stage anything.
