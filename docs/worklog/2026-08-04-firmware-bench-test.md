# Worklog - 2026-08-04

- **Date:** 2026-08-04
- **Phase:** 2

## Done

- Found 5x 47R resistors already on hand, separate from the in-transit
  30-value assortment. Measured 48R on a multimeter's 2k range [measured
  2026-08-04], consistent with the yellow-violet-black-gold color code
  (nominal 47R, 5% tolerance: 48R is within band). Logged in
  `.claude/inventory.yaml`.
- Fixed a local tooling gap: `pio` (PlatformIO's CLI) was installed
  (`platformio` 6.1.19 via `pip3`) but its binary at
  `~/Library/Python/3.9/bin/pio` wasn't on `PATH`, so a fresh terminal
  reported `command not found: pio`. Diagnosed with `pip3 show platformio`
  and `find`; fixed for the session with an `export PATH=...` line. A
  permanent fix (adding that line to `~/.zshrc`) was offered but left to the
  user to do themselves.
- Set up a throwaway Mosquitto broker (Homebrew, `mosquitto` 2.1.2) on the
  dev Mac for the bench test - not the Pi, and not the project's own
  `server/docker-compose.yml`. Docker Desktop on macOS doesn't give a
  container true host networking the way the Pi does, so the compose
  Mosquitto service wouldn't have been reachable from the ESP32 over WiFi;
  a natively-installed broker bound to the Mac's real LAN IP
  (`192.168.0.100`) sidesteps that. Config and password file live in
  `/tmp/gate-bench-mosquitto/` - outside the repo, ephemeral, not part of
  the deployment. `allow_anonymous false`, two credentialed clients
  (`gate-esp32`, `bench-tester`), matching the MQTT contract's broker rules.
- Wired the bench substitute for the relay: `GPIO27` (`RELAY_TRIGGER_PIN`)
  through one of the 47R resistors to an LED to `GND`, on the ESP32
  screw-terminal breakout board (`D27` and a `GND` terminal). Bench-only -
  no CP80, no gate, no mains involved.
- Flashed `firmware/` to the physical ESP32 (`pio run -e esp32dev -t
  upload`) and ran the full bench-test procedure from
  `firmware/README.md` / hard condition 5, driving it with
  `mosquitto_pub`/`mosquitto_sub` against the MQTT contract:
  - WiFi + MQTT connect confirmed; LWT registered on `gate/availability`,
    which published retained `online` on connect.
  - Trigger -> LED pulse -> `gate/cmd/ack` `result:"accepted"` confirmed,
    LED pulse visually confirmed (see Did Not Work - it's brief).
  - Duplicate `request_id` correctly rejected (`result:"duplicate"`) even
    ~170s after the original pulse's cooldown had already elapsed - this is
    the specific behavior the MQTT contract calls for (dedup must outlive
    cooldown, since a delayed QoS 1 redelivery can arrive late), not just a
    generic duplicate check.
  - A fresh `request_id` fired with no delay right after an accepted
    trigger correctly hit `rejected_cooldown`, while the first stayed
    `accepted`.
  - Periodic `gate/state` heartbeat observed arriving on its own, with no
    trigger sent, at the `STATE_HEARTBEAT_INTERVAL_MS` cadence.
  - LWT confirmed: unplugging the ESP32's USB cable caused the broker to
    log `disconnected: exceeded timeout` ~46s after the last `PINGREQ`, and
    push `gate/availability` -> `offline` to the subscriber.
  - This closes the firmware end-to-end bench-test item carried over as an
    explicit TODO in both `2026-07-29-firmware-skeleton.md` and
    `2026-07-30-server-skeleton-and-cloudflare-tunnel.md`, and satisfies
    hard condition 5's requirement to bench-test with an LED before any
    real CP80 wiring.

## Learned

- **The `gate/state` payload's reported `state` ("closed") during this test
  is meaningless**, not a bug: `STATUS_SENSE_PIN` (`GPIO34`) was wired to a
  stable bench level, not the real CP80 LED terminal, so the decoder's
  classification is arbitrary here. Matches the Phase 3 placeholders
  already documented in `firmware/README.md` and
  `docs/wiring/04-gpio-pin-map.md` - nothing to fix.
- A 500ms LED pulse is genuinely easy to miss by eye if attention is on a
  terminal window instead of the board at the exact moment a trigger is
  sent. Worth remembering for any future manual bench check that relies on
  watching an LED - a phone camera recording, or the multimeter, is a more
  reliable witness than a human glance.
- Confirmed the dedup-outlives-cooldown behavior the MQTT contract
  specifically calls for, by accident: the first duplicate test happened
  long after cooldown had elapsed and still correctly returned
  `"duplicate"`.

## Did Not Work

- First attempt at testing `rejected_cooldown` didn't actually exercise
  it: the second trigger was sent 4+ seconds after the first (typed by
  hand, with a duplicate-check command run in between), well outside the
  3s cooldown window, so it was legitimately `accepted` rather than
  rejected. Fixed by firing two fresh triggers back-to-back on one shell
  line (`;` with no delay) to land the second one inside the cooldown
  window.
- `pio run -e esp32dev -t upload` failed initially with `command not
  found: pio` - see Done above for the cause and fix.

## Measurements

- Resistor: 48R measured on a multimeter's 2k ohm range [measured
  2026-08-04], five of this value found on hand. Consistent with a 47R 5%
  color-coded resistor. Recorded in `.claude/inventory.yaml`, not a
  `docs/wiring/` document, since this isn't a CP80-side measurement.

## Next

- Install `cloudflared` on the Pi and connect it to `ZTA_TUNNEL_TOKEN_GATE`
  (Doppler) - still not done, no Pi access this session.
- Run `docker compose up` on the actual Pi 5 with real `server/.env` values
  and the Mosquitto password file - still not done. Only the bench-only
  native Mosquitto broker on the dev Mac was exercised this session, not
  the Pi-side Docker Compose stack.
- Firmware is now bench-proven end-to-end; real CP80 wiring is unblocked
  from the firmware-readiness side, but still blocked on the in-transit
  resistor assortment (needed for the actual status-sense divider, a
  different part from the 47R bench resistors found this session) and on
  the WiFi-inside-closed-housing RSSI check.
- The throwaway bench Mosquitto broker and its config live in
  `/tmp/gate-bench-mosquitto/` on the dev Mac - ephemeral (may not survive
  a reboot), not part of the deployment. Fine to regenerate on demand for
  any further bench iterations; nothing there needs preserving.
