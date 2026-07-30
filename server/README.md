# Gate Wireless Controller - Server

FastAPI service and Mosquitto broker for the Pi 5, described in the repo
root `CLAUDE.md`. Talks MQTT to the ESP32 per
`.claude/skills/mqtt-contract/SKILL.md`, and is the only thing on the Pi
reachable through the Cloudflare Tunnel (hard condition 3 in `CLAUDE.md`).
Deployment choice explained in
[docs/decisions/0004-docker-compose-over-systemd.md](../docs/decisions/0004-docker-compose-over-systemd.md).

## First-Time Setup

```sh
cp .env.example .env    # fill in MOSQUITTO_LAN_BIND_IP and MQTT_API_PASSWORD
```

Generate the Mosquitto password file - see
[mosquitto/README.md](mosquitto/README.md) for the exact commands and why
`gate-api`'s password here must match `MQTT_API_PASSWORD` in `.env`.

## Run

```sh
docker compose up --build
```

Both services run under `network_mode: host` (Linux only - this will not
behave the same under Docker Desktop on macOS/Windows). FastAPI listens on
`127.0.0.1:8000` only; Mosquitto listens on `MOSQUITTO_LAN_BIND_IP:1883`
only. Neither is reachable from the internet directly - remote access is
`cloudflared` (not yet set up) fronting FastAPI.

## Test (API only, no broker or Docker required)

```sh
cd api
python3.12 -m venv .venv && .venv/bin/pip install -e ".[dev]"
.venv/bin/pytest
.venv/bin/mypy src
.venv/bin/ruff check .
```

`tests/` mocks the MQTT client entirely (`tests/conftest.py`), so these run
with no broker, no Docker, and no gate.

## Known Gaps (Phase 2 Stage B, not yet done)

- Cloudflare Tunnel + Zero Trust Access policy - `cloudflared` is not part
  of this compose stack yet
- No structured retry/backoff tuning beyond the defaults in
  `api/src/gate_api/mqtt/client.py` - untested against a real flaky
  connection
- `docker compose build` / `up` has not been run against real Docker (not
  available in the development environment this skeleton was written in) -
  verify on the Pi, or on any machine with Docker, before relying on it
