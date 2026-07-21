# 0001 - Cloudflare Tunnel over Tailscale

- **Date:** 2026-07-21
- **Phase:** 3 (affects design now, implemented later)
- **Status:** verified

## Context

The Pi 5 runs the FastAPI service that fronts the gate. The phone needs to
reach it from anywhere. Hard condition 3 originally read "nothing is exposed to
the public internet", written with Tailscale in mind.

The project is switching to Cloudflare. The two approaches are not equivalent
on that condition, so the condition had to be reworded rather than
find-and-replaced.

## Options Considered

**Tailscale (original plan).** Private WireGuard mesh. No public DNS record
exists at all. Simplest possible reading of hard condition 3. Requires the
Tailscale client running on the phone.

**Cloudflare Tunnel + Zero Trust Access (chosen).** `cloudflared` on the Pi
opens an outbound-only tunnel. A hostname resolves publicly, but an Access
policy at Cloudflare's edge rejects unauthenticated requests before they reach
home. No client app strictly required on the phone.

**Cloudflare Tunnel alone, auth in FastAPI.** Simpler to set up, but the
application code becomes the only barrier between the open internet and a
physical gate. Any auth bug is directly reachable and the blast radius is the
gate opening.

**Cloudflare WARP private network.** Cloudflare's closest equivalent to
Tailscale. Keeps hard condition 3 nearly as originally written, but reintroduces
a client app on the phone, which was part of what the move away from Tailscale
was meant to avoid.

## Decision

Cloudflare Tunnel with Zero Trust Access in front.

The deciding factor against tunnel-alone: authentication must not live in
project code. Access rejects unauthenticated traffic at Cloudflare's edge, so a
bug in the FastAPI auth path cannot by itself expose the gate.

## Consequences

- Hard condition 3 is rewritten. The absolute "nothing exposed publicly" is
  replaced by "no inbound ports, and no unauthenticated route to the API". The
  tunnel hostname does resolve publicly, and pretending otherwise would hide a
  real part of the threat model
- FastAPI binds to localhost only. It must be reachable through `cloudflared`
  and nothing else. A service bound to `0.0.0.0` would silently undo the
  protection, so this is worth checking explicitly during Phase 3
- The Flutter app must carry an Access credential: a service token, or the
  Access browser login flow. This is new Phase 4 work that Tailscale would not
  have required
- New dependency on Cloudflare as a availability single point of failure. If
  Cloudflare or the domain has a problem, remote gate access is down. The NOVA
  remotes still work at short range, so this is an inconvenience rather than a
  lockout
- Tunnel credentials and the tunnel UUID are secrets. They must not enter
  `docs/`, `.tf` files, or any tracked config

## Follow-Ups

- [ ] Confirm the Access policy rejects unauthenticated requests before Phase 4
      app work begins
- [ ] Decide between service token and Access login flow for the app
- [ ] Verify FastAPI binds to localhost only once the server exists
