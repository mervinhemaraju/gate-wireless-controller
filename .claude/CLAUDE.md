# Gate Wireless Controller

## Problem Statement

I have a Centurion D3 sliding gate motor at home with two small NOVA remote
controllers (one stays at home, one stays in my car). The remotes use 433MHz
rolling-code (KeeLoq) RF, so they only work within short range of the gate.
When I am away from home I cannot open or close the gate at all.

## Goal

Control the gate (open / close / status) from my phone, from anywhere over
the internet, using hardware I control end to end.

## Approach

An ESP32 is wired directly to the D3's CP80 control board and acts as the
on-gate agent. It talks wirelessly over home WiFi (MQTT) to a Raspberry Pi 5
indoors, which exposes an API reachable from my phone through a Cloudflare
Tunnel with Zero Trust Access in front of it. A simple Flutter app is the
front end.

```
Phone (anywhere)
   |  HTTPS to the tunnel hostname
   v
Cloudflare edge
   |  Zero Trust Access policy: unauthenticated requests
   |  are rejected here, before reaching home
   v
Raspberry Pi 5 (indoors, always powered)
   |  cloudflared (outbound tunnel only, no inbound ports)
   |  FastAPI service (bound to localhost) + Mosquitto MQTT broker
   |
   |  home WiFi
   v
ESP32 (inside the D3 motor housing)
   |  short low-voltage wires
   v
D3 control board (CP80)
   - TRG to COM pulse  -> trigger (open / stop / close)
   - Status LED output -> gate state feedback (decoded flash pattern)
   - 12V aux + buck    -> powers the ESP32 (battery-backed, survives power cuts)
```

## Hard Conditions (never violate these)

1. The Raspberry Pi must NEVER be wired to the gate motor. The link between
   the Pi and the gate side is wireless only.
2. Wiring directly to the motor itself is allowed, but only via the ESP32,
   and only on the low-voltage (12V) side of the CP80 board.
3. No inbound ports, ever. No port forwarding, no UPnP, no DMZ. Remote access
   is an outbound-only Cloudflare Tunnel, and every request must be
   authenticated by Zero Trust Access at Cloudflare's edge before it reaches
   the Pi. The tunnel hostname resolves publicly, so there must never be an
   unauthenticated route to the API. FastAPI binds to localhost only: it is
   reachable through cloudflared and nothing else.
4. The existing NOVA remotes must keep working unchanged.
5. Mains power and the battery must be disconnected before any wiring work
   on the motor.

## Project Rules

These sit on top of the global rules in `~/.claude/rules/`.

Always in force:

@rules/phase-gate.md
@rules/inventory-sync.md
@rules/repo-layout.md
@rules/docs-conventions.md
@rules/wiring-research.md

## Project Skills

Workflow:

- `/whats-next [area]` - everything that could be worked on right now
- `/log-progress [notes]` - end of session: worklog, status, inventory
- `/inventory-update [parts]` - move arrived parts into `already_have`
- `/d3-manual <topic>` - look something up in the scanned D3/D5 manual
- `/wiring-check [work]` - pre-flight safety checklist before Phase 3 wiring

Build standards, loaded on demand to keep them out of every turn's context.
**These are mandatory, not optional reading:**

- `/esp32-firmware [task]` - **load before any work under `firmware/`**,
  before choosing GPIO pins, and before touching the relay or LED decoding
- `/mqtt-contract [task]` - **load before any MQTT code, gate-state handling,
  trigger endpoint, or Mosquitto config**, in firmware, server or app alike

Skipping these is how the gate ends up opening itself at boot, or the app ends
up reporting a state the gate is not in.

## Phases

Work happens in strict phases. Focus is getting a working system first;
enhancements come last.

- **Phase 1 - Buy the parts.** Everything needed is listed in
  `inventory.yaml`. Also verify home WiFi actually reaches the gate motor:
  stand at the motor with a phone on home WiFi and check the signal. Note
  that the 433MHz remotes working from far away proves nothing about 2.4GHz
  WiFi range. Link options, in order of preference:
  1. Router WiFi reaches the gate: ESP32 joins the home network (simplest).
  2. Pi 5 as WiFi access point: the ESP32 connects directly to a small
     network hosted on the Pi's radio; Pi gets internet over Ethernet.
  3. ESP-NOW bridge: second ESP32 on the Pi's USB as a serial bridge
     (longest range, most custom code, last resort).
- **Phase 2 - Software (built against the contract).** ESP32 firmware (MQTT:
  publish state, subscribe to commands), the status decoder state machine,
  Mosquitto + FastAPI service on the Pi, cloudflared tunnel and Zero Trust
  Access policy. Everything here is designed against the MQTT contract and
  bench-tested without the gate. Two items stay as explicit TODOs because they
  can only be pinned down against real hardware in Phase 3: the LED flash-code
  to gate-state mapping, and the final GPIO pin assignments (relay polarity has
  to be measured first).
- **Phase 3 - Wiring, hardware characterisation and integration.** Wire the
  ESP32 to the CP80 board: trigger relay/optocoupler on TRG-COM, status LED
  sense line, buck converter on the 12V aux output. Measure the 12V aux rail,
  the status LED voltage and polarity, and the relay polarity; fill in the
  decoder mapping and pin assignments left open in Phase 2. Bench-test, then
  end-to-end test: API call opens the gate and state is reported back.
- **Phase 4 - Flutter app.** A simple app: one trigger button plus live gate
  state, talking to the Pi's API through the Cloudflare Tunnel. The app has to
  carry a Cloudflare Access service token (decided in ADR 0001).
- **Phase 5 - Improvements (later, not now).** Examples: notification when
  the gate stays open too long, open/close history, pedestrian-mode button,
  reed switch for hard closed-position confirmation. Do not start any of
  this until Phases 1-4 are done and the system works.

## Current Status

- Phase 1 procurement complete: the 6-colour hookup wire arrived 2026-07-28,
  the last outstanding part. `to_buy` and `in_transit` are both empty. Nothing
  in Phase 3 wiring is blocked on parts any more.
- Phases reordered: software is now Phase 2, physical wiring plus integration
  is Phase 3. This lets the firmware, MQTT contract, FastAPI service and
  Cloudflare/ZTA setup proceed now while the wire is in transit. The end-to-end
  "API call opens the gate" test stays in Phase 3 because it needs the wiring.
- WiFi at the gate verified: good signal at the motor (phone test, open air).
  Caveat: the ESP32 will sit inside the motor housing, which may attenuate
  the signal; confirm actual RSSI from inside the closed housing on install
  day. Fallbacks if weak: external antenna (U.FL), Pi 5 as access point,
  ESP-NOW bridge. Link option 1 (router WiFi) remains the plan.

## Planned Repository Layout

- `.claude/rules/` - project-specific rules, imported above
- `.claude/skills/` - project-specific slash commands
- `inventory.yaml` - full parts list: what I own, what to buy
- `firmware/` - ESP32 firmware (Phase 2)
- `server/` - Python FastAPI service + MQTT config for the Pi (Phase 2)
- `app/` - Flutter app (Phase 4)
- `docs/` - all written work: decisions, wiring, worklog, reference,
  runbooks. Conventions in `.claude/rules/docs-conventions.md`

## Reference Material

- D3/D5 installation manual (CP80 terminals, status LED flash codes):
  https://www.centsys.co.za/upload/0_07_A_0115_%20D3D5%20installation%20manual%2022072013-BM-for%20web.pdf
- NOVA remote system (why RF cloning is not an option):
  https://www.centsys.co.za/nova/
