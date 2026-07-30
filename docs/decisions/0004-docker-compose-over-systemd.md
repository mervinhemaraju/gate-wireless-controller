# 0004 - Docker Compose over Bare venv + systemd

- **Date:** 2026-07-30
- **Phase:** 2
- **Status:** verified

## Context

The FastAPI service and Mosquitto broker need to run persistently on the
Pi 5. Two ways to do that: containerized under Docker Compose, or a Python
venv plus a native Mosquitto install, both managed by systemd units.

## Options Considered

**Docker Compose.** Both services as pinned, non-root, multi-stage
containers per `~/.claude/rules/docker-kubernetes.md`. Isolated dependency
trees, reproducible builds, one `docker compose up` to redeploy. Overhead:
Docker itself running on the Pi, and container networking has to be worked
out to satisfy the MQTT contract's interface-binding requirements (see
Consequences).

**Bare venv + systemd.** FastAPI in a venv under a systemd unit, Mosquitto
as a native Debian package/service. Fewer moving parts on a single-purpose
Pi, no container runtime overhead, direct access to host network interfaces
with no NAT to reason about. Cost: manual dependency management, no
isolation between the two services, redeploys mean re-activating the venv
and restarting units by hand.

## Decision

Docker Compose, both services under `network_mode: host`.

`network_mode: host` rather than Docker's default bridge network: two
contract requirements depend on binding to a specific host interface -
Mosquitto to the LAN IP only, FastAPI to `127.0.0.1` only (hard condition 3)
- and bridge networking's NAT means the container's internal bind address is
not the same as what's reachable from the host or LAN. Host networking
sidesteps that entirely: what the process binds to inside the container is
exactly what's reachable outside it, matching `.claude/skills/mqtt-contract/SKILL.md`
and hard condition 3 to the letter rather than by an approximation.

## Consequences

- Host networking is Linux-only. Fine for the Pi 5's Docker Engine, but this
  compose file will not behave the same way on Docker Desktop (macOS/Windows)
  - development/testing of the containers themselves has to happen on Linux
    or by reasoning through the Dockerfiles directly, not by running the
    stack locally on a Mac
- No container-to-container Docker DNS (e.g. `mosquitto` as a hostname) is
  available under host networking, since both containers share the Pi's
  network namespace directly. `gate-api` reaches the broker via
  `MOSQUITTO_LAN_BIND_IP` like any other LAN client would, not by service
  name
- Mosquitto's password file (`server/mosquitto/passwords/`) is generated
  once, locally, before first start - see `server/mosquitto/README.md`. Not
  automated, by design: rotating it should be a deliberate action, not
  something a redeploy silently does
- Two more Dockerfiles and a compose file to keep pinned and patched, versus
  systemd units with no image layer at all. Accepted as the smaller ongoing
  cost against the isolation and reproducibility gained
