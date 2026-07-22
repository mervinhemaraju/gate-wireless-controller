# Phase Gate: Do Not Skip Ahead

`CLAUDE.md` states that work happens in strict phases. Before proposing or
starting any Phase N work, confirm Phase N-1 is actually complete.

## How to Check

Read the `## Current Status` section of `.claude/CLAUDE.md` for what is done,
and the `## Phases` section for what each phase has to deliver.

A phase is complete when its deliverable is met and verified, not when it feels
mostly done. Status text can go stale, so where `## Current Status` and the
evidence disagree (inventory, `docs/worklog/`, what exists in the repo), trust
the evidence and say the status line needs updating.

## If the Previous Phase Is Not Complete

- Do not start the Phase N work
- Say which phase is still open and what specifically is left in it
- Offer to help close the current phase instead

## The Desk-Work Exception

Phase 2 is software and proceeds against the MQTT contract rather than on
verified hardware. That is the plan, not skipping ahead. It includes:

- The MQTT contract and payload schemas
- The status decoder state machine and its `native` unit tests
- The FastAPI skeleton, and the Cloudflare Tunnel and Access policy design
- Documentation and decision records

What may **not** be finalised in Phase 2 is anything whose correctness depends
on unverified hardware behaviour: mapping specific LED flash patterns to gate
states before those patterns have been observed on the real gate, fixing pin
assignments before the relay polarity is measured, or anything that would have
to be redone once the hardware is characterised. These belong to Phase 3, where
the gate is wired and measured; leave them as explicit TODOs in Phase 2.

When a Phase 2 item is deferred to Phase 3 for this reason, say so explicitly
rather than quietly guessing the hardware behaviour.

## Why This Matters

A half-assembled phase produces states that are very hard to debug, because a
failure could be in either layer. Writing the Flutter app against an API whose
gate-state reporting has never been proven against real hardware means a bug
could be in the app, the API, the firmware decoder, or the wiring, with no way
to bisect. Each phase deliverable is a verified checkpoint; skipping one hides
an assumption that surfaces later at much higher cost.

Phase 5 is explicitly deferred. Capture the ideas, start none of them until
Phases 1 to 4 work end to end.
