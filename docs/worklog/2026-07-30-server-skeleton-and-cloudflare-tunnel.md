# Worklog - 2026-07-30

- **Date:** 2026-07-30
- **Phase:** 2

## Done

- Corrected `.claude/CLAUDE.md`'s `## Current Status`: the resistor
  assortment had been marked arrived in an earlier inventory edit, but it is
  still `in_transit` - confirmed with the user and fixed the status line to
  stop claiming Phase 1 procurement is fully closed.
- Built the Phase 2 Stage B server skeleton in `server/` (FastAPI + Mosquitto,
  Docker Compose, per `docs/decisions/0004-docker-compose-over-systemd.md`):
  - `server/api/` - FastAPI service (`src/gate_api/`): MQTT client with
    reconnect/backoff, in-memory state cache, `request_id`-correlated
    trigger/ack flow, `GET /health`, `GET /state`, `POST /trigger`, all
    checked against `.claude/skills/mqtt-contract/SKILL.md`. 17 tests
    (routes mocked against a fake MQTT client, real client's parsing/
    correlation logic tested standalone), all passing; clean on
    `mypy --strict`, `ruff`, `black`, `isort`.
  - `server/mosquitto/` - pinned `eclipse-mosquitto:2.0.18`,
    `allow_anonymous false`, listener bound to the LAN interface via an
    `envsubst`-rendered config.
  - `server/docker-compose.yml` - both services on `network_mode: host` so
    Mosquitto can bind the LAN IP and FastAPI can bind `127.0.0.1` literally,
    satisfying hard condition 3 without bridge-network NAT in the way.
  - ADR `0004` written for the Docker Compose vs. bare venv+systemd choice.
- Added `.github/workflows/docker-build.yml`: builds and pushes `gate-api`
  and `gate-mosquitto` to GHCR, tagged `dev` on push to `dev` and `prod` on
  push to `main`. **Verified working** - both images pushed successfully on
  the first real run. This also closes the "Docker build unverified" gap
  flagged earlier in the session (no Docker available in the dev
  environment): the CI run is real proof both Dockerfiles build clean.
- Planned and implemented the Cloudflare Tunnel + Zero Trust Access side in
  the separate `cloudflare` repo (`components/200-zta`, `components/100-dns`)
  - **applied successfully to real infrastructure**, closing the last
    `[ ]` Follow-Up item in `docs/decisions/0001-cloudflare-tunnel-over-tailscale.md`:
  - `200-zta/gate_tunnel.tf` - `cloudflare_zero_trust_tunnel_cloudflared`
    (remotely managed, `config_src = "cloudflare"`) + its ingress config
    (`gate.mervinhemaraju.com` -> `http://127.0.0.1:8000`, catch-all
    `http_status:404`) + the connector token data source.
  - `200-zta/zta_apps.tf` - reworked at the user's direction into a flat
    list of per-domain maps (`{ domain = "...", sso = bool, policies = [...] }`),
    referencing policy resources directly rather than through a string-keyed
    lookup map. Every app gets the shared Monitoring service-token bypass by
    default (`apps.tf`, precedence 1); `policies` is extra policy IDs on top
    of that. The gate's entry: `sso = false`, extra policy =
    `gate_service_token` (its own dedicated token, kept separate from
    Monitoring so rotating one never affects the other).
  - `200-zta/service_token.tf`, `policies.tf`, `secrets.tf` - dedicated
    `gate` service token, `gate_service_token` Access policy, both pushed to
    Doppler (`ZTA_SERVICE_TOKEN_GATE`, `ZTA_TUNNEL_TOKEN_GATE`) for the Pi's
    `cloudflared` install and the future Flutter app to read.
  - `100-dns/datasources.tf` + `dns_records_mh.tf` - looks the tunnel up by
    name (not by reading `200-zta`'s state directly, matching this repo's
    existing per-component independent-lookup convention) and adds the
    proxied CNAME `gate.mervinhemaraju.com` -> `<tunnel-id>.cfargotunnel.com`.

## Learned

- **A Terraform conditional whose result depends on comparing against a
  not-yet-applied resource's `.id` can break the provider on first apply.**
  `allowed_idps = contains(each.value.policies, cloudflare_zero_trust_access_policy.access.id) ? [...] : []`
  produced "Received unknown value, however the target type cannot handle
  unknown values" against the Set-typed `allowed_idps` attribute - the
  ternary's outcome was itself unknown at plan time on a fresh apply. Fixed
  by replacing it with a plain static `sso` boolean per app entry instead of
  deriving the decision from an unapplied resource's computed ID. Worth
  remembering for any future per-item conditional built from a `for_each`
  over resource-referencing data in this or other Terraform repos.
- **`data "cloudflare_zero_trust_tunnel_cloudflared"`'s name filter is
  nested under a `filter` block**, not a top-level `name` argument - the
  fetched provider doc (rendered via a summarizing fetch, not the raw
  schema) got this wrong. `tofu validate` caught it immediately; the fix was
  `filter = { name = "..." }`. General lesson: a doc *summary* can be wrong
  even when doc-confirmed per `~/.claude/rules/terraform.md` - validate
  against the actual provider schema when a summary and reality disagree,
  don't just trust the fetch.
- **The Cloudflare API token in Doppler (`CLOUDFLARE_TERRAFORM_TOKEN`)
  didn't have the Tunnel permission scope**, since nothing in either repo had
  touched the Tunnels API before today. Got a `403` / error code `10000`
  ("Authentication error") on `POST .../cfd_tunnel` until the user added
  **Account -> Cloudflare Tunnel -> Edit** to the token's permissions in the
  Cloudflare dashboard. Worth remembering if this token is ever rotated.

## Did Not Work

- First `gate_app.tf` draft: a standalone `cloudflare_zero_trust_access_application`
  resource outside the shared `protected_apps` for_each loop. User rejected
  this - all ZTA apps belong in `zta_apps.tf`, restructured as a dict/list
  instead of a one-off resource. Superseded by the `protected_apps` list
  entry described above.
- Second draft used a `local.policy_catalog` string-key lookup map
  (`policy_key -> policy.id`) to keep `zta_apps.tf` free of direct resource
  references. User rejected this too - direct resource references only, no
  indirection layer. Removed `policy_catalog` entirely.
- First `100-dns` tunnel lookup used `name` as a top-level data source
  argument - wrong schema, see Learned above.
- First `gate_tunnel.tf` draft considered generating a `tunnel_secret` via
  a `random` provider resource for a locally-managed tunnel. Not needed:
  `config_src = "cloudflare"` (remotely managed) uses a connector token
  instead, avoiding a new provider dependency entirely.

## Measurements

None this session - entirely desk/software work (server code, CI, Terraform).

## Next

- Install `cloudflared` on the Pi and point it at `ZTA_TUNNEL_TOKEN_GATE`
  (Doppler) - not done this session, no Pi access involved.
- Run `docker compose up` on the actual Pi 5 with real `server/.env` values
  and the Mosquitto password file (`server/mosquitto/README.md`) - the
  compose stack has only been validated by CI building the images, not by
  actually running them together.
- Carried over from 2026-07-29, still not done: bench-test the full
  firmware with an LED standing in for the relay, confirming the MQTT
  round-trip end to end, before any real CP80 wiring (`firmware/README.md`,
  hard condition 5).
- Wire the Flutter app to send `CF-Access-Client-Id` /
  `CF-Access-Client-Secret` from `ZTA_SERVICE_TOKEN_GATE` - Phase 4 work,
  now unblocked on the Cloudflare side.
- Phase 2 Stage B is close to done but not fully closed: FastAPI, Mosquitto,
  CI, and the Cloudflare Tunnel/Access side are all built and (for
  Cloudflare) applied to real infrastructure, but nothing has been proven
  end-to-end (Pi running the stack, `cloudflared` connected, firmware
  round-tripping through it) yet.
