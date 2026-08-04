# Worklog - 2026-08-05

- **Date:** 2026-08-05
- **Phase:** 2

## Done

- Installed `cloudflared` on the Pi 5 and connected it to the already-applied
  Cloudflare Tunnel (`ZTA_TUNNEL_TOKEN_GATE`, Doppler project
  `cloudflare-creds`, config `prd`) via `cloudflared service install
  <token>`, running as a systemd service. Confirmed `active (running)` and
  healthy precheck.
- Cloned `gate-wireless-controller` onto the Pi and brought up `server/`'s
  Docker Compose stack against real Docker for the first time (previously
  only verified by CI building the images, per `server/README.md`'s "Known
  Gaps"):
  - Configured `server/.env`: `MOSQUITTO_LAN_BIND_IP` and `MQTT_BROKER_HOST`
    both set to the Pi's real LAN IP (`192.168.0.3`, identified via
    `hostname -I` - the Pi also showed two Docker bridge-gateway addresses,
    `172.17.0.1`/`172.18.0.1`, which are not reachable from the LAN and were
    correctly ignored).
  - Generated the Mosquitto password file (`gate-api`, `gate-esp32`
    credentials) per `server/mosquitto/README.md`.
  - `docker compose up --build -d`: both containers came up `healthy`.
    `curl http://127.0.0.1:8000/health` (run on the Pi, since FastAPI binds
    `127.0.0.1` only) returned `{"status":"ok","mqtt_connected":true}`.
- Verified the full remote path through the real Cloudflare Tunnel:
  - Unauthenticated `curl https://gate.mervinhemaraju.com/health` correctly
    blocked/redirected by Zero Trust Access rather than reaching FastAPI.
  - Authenticated request (`CF-Access-Client-Id`/`CF-Access-Client-Secret`
    headers from the `ZTA_SERVICE_TOKEN_GATE` JSON secret, same Doppler
    project/config) returned `200` with the healthy body - proving the
    phone-to-gate path Cloudflare edge -> Zero Trust -> tunnel -> FastAPI ->
    Mosquitto works end to end, ahead of the Flutter app existing.
- Wrote a redo-from-scratch runbook in Notion ("Raspberry Pi Setup" ->
  "Gate Wireless Controller - Server & Tunnel Setup") covering all of the
  above plus the two gotchas below, for if the Pi ever needs rebuilding.
- Fixed `server/.env.example`'s `MQTT_BROKER_HOST` default at the source
  (was `mosquitto`, now `192.0.2.10` matching `MOSQUITTO_LAN_BIND_IP`'s
  placeholder, with a comment explaining why) - same day as found, so the
  next person copying the example won't hit the silent-reconnect-loop bug
  described below.
- Combined with yesterday's firmware bench test
  (`2026-08-04-firmware-bench-test.md`), this closes **Phase 2 Stage B in
  full**, and with it Phase 2 overall - firmware, server, and the Cloudflare
  side are all built and verified. The two placeholders that were always
  scoped to wait for Phase 3 (status-sense polarity, LED flash-code mapping)
  remain open by design, not as a gap in Phase 2's completion.

## Learned

- **Cloudflare's `apt` package repo lags behind Debian's newest release
  codenames.** `apt-get install cloudflared` 404'd on Raspberry Pi OS's
  `trixie` codename (`trixie Release` doesn't exist on their repo yet).
  Fixed by downloading the `.deb` directly from Cloudflare's GitHub releases
  (`cloudflared-linux-arm64.deb`) and `dpkg -i`-ing it instead of going
  through `apt`. Worth trying this first on any future bleeding-edge-OS Pi
  setup rather than debugging the repo.
- **`server/.env.example`'s `MQTT_BROKER_HOST=mosquitto` default is wrong**
  for this deployment, and was only caught by actually running the stack,
  not by review. ADR 0004 already documents that `network_mode: host` gives
  up container-to-container Docker DNS - `gate-api` has to reach the broker
  by its real LAN IP, the same as any other client, not by service name.
  The example file was never updated to reflect that when ADR 0004 was
  written. Left as a real fix still to make (see Next) rather than papered
  over.

## Did Not Work

- `sudo apt-get install cloudflared` failed with `404 Not Found` fetching
  `https://pkg.cloudflare.com/cloudflared trixie Release` - see Learned
  above for cause and fix.

## Measurements

None this session - Pi/software deployment work only, no electrical
measurements.

## Next

- Phase 3 physical wiring is now unblocked from the software-readiness side
  entirely. Remaining blockers unchanged from before: the in-transit
  resistor assortment for the real status-sense divider, and the
  WiFi-inside-closed-housing RSSI check on install day.
- Phase 4 (Flutter app) is now genuinely unblocked on the Cloudflare/API
  side - the exact request shape (`CF-Access-Client-Id`/`Secret` headers
  plus `GET /health` and friends) that the app will need to replicate has
  been proven working end to end this session.
