# Documentation Index

All written work on the gate wireless controller. Conventions (naming,
ordering, required headers) are in `.claude/rules/docs-conventions.md`.

Every document added here needs a line in this index.

## decisions/

Architecture and component decisions, one per file, numbered in the order they
were made.

- [0001 - Cloudflare Tunnel over Tailscale](decisions/0001-cloudflare-tunnel-over-tailscale.md) - remote access method for the Pi's API
- [0002 - Resistor Divider over Optocoupler](decisions/0002-resistor-divider-over-optocoupler.md) - why the PC817 module was dropped for the gate-status sense line
- [0003 - Open Mounting Box](decisions/0003-open-mounting-box.md) - why the IP67 box is mounted open, D3 housing as the only environmental barrier
- [0004 - Docker Compose over Bare venv + systemd](decisions/0004-docker-compose-over-systemd.md) - why the Pi-side stack is containerized, and why both services run under `network_mode: host`

## wiring/

Pinouts, measured values and diagrams. Every electrical claim carries its
source.

- [01 - Optocoupler Front End](wiring/01-optocoupler-front-end.md) - PC817 module bench test (superseded, module rejected, see ADR 0002)
- [02 - Relay Module](wiring/02-relay-module.md) - HW-803 relay bench test: confirmed active-HIGH trigger polarity, single-channel correction
- [03 - Mounting Layout](wiring/03-mounting-layout.md) - dry-layout check: ESP32, relay and buck converter fit confirmed; box will be mounted open
- [04 - GPIO Pin Map](wiring/04-gpio-pin-map.md) - relay trigger and status sense pin assignments, with reasoning

## worklog/

Session by session record of what was done, learned and broken.

- [2026-07-28 - Optocoupler Bench Test](worklog/2026-07-28-optocoupler-bench-test.md) - Phase 1 procurement closed; PC817 module bench-tested and rejected
- [2026-07-28 (b) - Relay Bench Test](worklog/2026-07-28-relay-bench-test-b.md) - relay module bench-tested: active-HIGH trigger confirmed, single-channel correction
- [2026-07-29 - Firmware Skeleton](worklog/2026-07-29-firmware-skeleton.md) - mounting/pin-map close-out, first firmware code, MQTT contract amendment, verified with pio test/run
- [2026-07-30 - Server Skeleton and Cloudflare Tunnel](worklog/2026-07-30-server-skeleton-and-cloudflare-tunnel.md) - FastAPI + Mosquitto Docker Compose skeleton, GHCR CI, and the Cloudflare Tunnel + Zero Trust Access setup (separate `cloudflare` repo) applied to real infrastructure
- [2026-08-04 - Firmware Bench Test](worklog/2026-08-04-firmware-bench-test.md) - full ESP32 firmware bench-tested end-to-end with an LED standing in for the relay, against a throwaway local Mosquitto broker; closes the pre-CP80-wiring bench-test TODO
- [2026-08-05 - Pi Deployment and Tunnel](worklog/2026-08-05-pi-deployment-and-tunnel.md) - cloudflared and the Docker Compose stack brought up for real on the Pi 5, phone-to-gate path proven end to end through the Cloudflare Tunnel; closes Phase 2

## reference/

Extracted third-party material, mainly OCR output from the scanned D3/D5
manual (see `/d3-manual`).

- [d3-manual-p31-p41-cp80-terminals](reference/d3-manual-p31-p41-cp80-terminals.md) - CP80 terminal ident, order and functions (COM, 12V, TRG, LED status output, etc.)
- [d3-manual-p45-led-indicator-lights](reference/d3-manual-p45-led-indicator-lights.md) - STATUS LED flash-code table and the other indicator LEDs
- [d3-manual-p10-specifications](reference/d3-manual-p10-specifications.md) - D3 electrical specs and the aux-rail power budget

## runbooks/

How to operate and recover the system once it exists. Nothing here yet.
