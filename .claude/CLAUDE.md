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
indoors, which exposes a private API reachable from my phone via Tailscale.
A simple Flutter app is the front end.

```
Phone (anywhere)
   |  internet, via Tailscale (no public exposure)
   v
Raspberry Pi 5 (indoors, always powered)
   |  FastAPI service + Mosquitto MQTT broker
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
3. Nothing is exposed to the public internet. Remote access goes through
   Tailscale; no port forwarding, no public endpoints.
4. The existing NOVA remotes must keep working unchanged.
5. Mains power and the battery must be disconnected before any wiring work
   on the motor.

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
- **Phase 2 - Connect the parts to the motor.** Wire the ESP32 to the CP80
  board: trigger relay/optocoupler on TRG-COM, status LED sense line, buck
  converter on the 12V aux output. Bench-test before touching the gate.
- **Phase 3 - Code and configuration.** ESP32 firmware (MQTT: publish state,
  subscribe to commands), Mosquitto + FastAPI service on the Pi, Tailscale
  setup. End-to-end test: API call opens the gate and state is reported back.
- **Phase 4 - Flutter app.** A simple app: one trigger button plus live gate
  state, talking to the Pi's API over Tailscale.
- **Phase 5 - Improvements (later, not now).** Examples: notification when
  the gate stays open too long, open/close history, pedestrian-mode button,
  reed switch for hard closed-position confirmation. Do not start any of
  this until Phases 1-4 are done and the system works.

## Current Status

- Phase 1 in progress: inventory drawn up, parts not yet ordered.
- WiFi at the gate verified: good signal at the motor (phone test, open air).
  Caveat: the ESP32 will sit inside the motor housing, which may attenuate
  the signal; confirm actual RSSI from inside the closed housing on install
  day. Fallbacks if weak: external antenna (U.FL), Pi 5 as access point,
  ESP-NOW bridge. Link option 1 (router WiFi) remains the plan.

## Planned Repository Layout

- `inventory.yaml` - full parts list: what I own, what to buy
- `firmware/` - ESP32 firmware (Phase 3)
- `server/` - Python FastAPI service + MQTT config for the Pi (Phase 3)
- `app/` - Flutter app (Phase 4)
- `docs/` - wiring diagrams, D3 manual references, decisions

## Reference Material

- D3/D5 installation manual (CP80 terminals, status LED flash codes):
  https://www.centsys.co.za/upload/0_07_A_0115_%20D3D5%20installation%20manual%2022072013-BM-for%20web.pdf
- NOVA remote system (why RF cloning is not an option):
  https://www.centsys.co.za/nova/
