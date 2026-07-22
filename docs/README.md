# Documentation Index

All written work on the gate wireless controller. Conventions (naming,
ordering, required headers) are in `.claude/rules/docs-conventions.md`.

Every document added here needs a line in this index.

## decisions/

Architecture and component decisions, one per file, numbered in the order they
were made.

- [0001 - Cloudflare Tunnel over Tailscale](decisions/0001-cloudflare-tunnel-over-tailscale.md) - remote access method for the Pi's API

## wiring/

Pinouts, measured values and diagrams. Every electrical claim carries its
source. Nothing here yet: Phase 3 (wiring) has not started.

## worklog/

Session by session record of what was done, learned and broken. Nothing here
yet.

## reference/

Extracted third-party material, mainly OCR output from the scanned D3/D5
manual (see `/d3-manual`).

- [d3-manual-p31-p41-cp80-terminals](reference/d3-manual-p31-p41-cp80-terminals.md) - CP80 terminal ident, order and functions (COM, 12V, TRG, LED status output, etc.)
- [d3-manual-p45-led-indicator-lights](reference/d3-manual-p45-led-indicator-lights.md) - STATUS LED flash-code table and the other indicator LEDs
- [d3-manual-p10-specifications](reference/d3-manual-p10-specifications.md) - D3 electrical specs and the aux-rail power budget

## runbooks/

How to operate and recover the system once it exists. Nothing here yet.
