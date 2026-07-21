# Documentation Conventions

Every piece of work on this project gets written down in `docs/` at the repo
root. This build runs in phases across months, with long gaps between
sessions; anything not written down is lost by the next one.

Applies whenever work is finished, a decision is made, a measurement is taken,
or something is learned the hard way.

## Structure

```
docs/
  README.md              index of everything below, kept current
  decisions/             why a path was chosen (ADRs)
  wiring/                pinouts, diagrams, measured values
  worklog/               what was done, session by session
  reference/             extracted third-party material
  runbooks/              how to operate and recover the system
```

## Naming

- **Always kebab-case**, lowercase, `.md`. Never spaces, never underscores
- `decisions/` uses a zero-padded sequence, never reused or renumbered:
  `0001-cloudflare-tunnel-over-tailscale.md`
- `worklog/` uses an ISO date first so files sort chronologically:
  `2026-07-21-inventory-arrival.md`. Two sessions in one day get a `-b` suffix
- `wiring/` and `runbooks/` use a zero-padded sequence to force reading order:
  `01-cp80-terminals.md`, `02-power-and-buck.md`
- `reference/` mirrors the source: `d3-manual-p23-terminals.md`
- Names describe content, not status. Never `notes.md`, `misc.md`, `temp.md`,
  `final-v2.md`

## Every Document Starts the Same Way

```markdown
# CP80 Terminal Identification

- **Date:** 2026-07-21
- **Phase:** 2
- **Status:** verified | provisional | superseded by [0004](0004-x.md)
```

`provisional` is the honest default for anything not yet confirmed against
hardware. Mark superseded documents at the top and link forward; never delete
a wrong document, because the reasoning behind a reversal is worth keeping.

## Decisions (ADRs)

One decision per file, written when the decision is made rather than later:

- **Context:** what forced the choice
- **Options considered:** each with its real trade-off
- **Decision:** what was chosen
- **Consequences:** what this now commits the project to, including the bad
  parts

The Cloudflare-over-Tailscale switch is exactly this kind of decision, along
with the WiFi link option, the relay-versus-transistor choice, and the MQTT
topic design.

## Wiring Documents

Held to `.claude/rules/wiring-research.md`: every electrical claim carries its
source inline (`[manual p.23]`, `[measured 2026-07-21]`, `[UNVERIFIED]`).

- Record the measured value, the date, and the conditions it was measured under
- ASCII diagrams inline, so the documentation survives without an image viewer
- Photographs go in `docs/wiring/img/`, referenced from the markdown, named for
  what they show

## Worklog Entries

Written at the end of a working session by `/log-progress`:

- What was done, concretely
- What was learned, especially anything that contradicted an assumption
- What broke or did not work, kept in rather than tidied away
- What is next, and anything now blocked

## Keep the Index Current

`docs/README.md` lists every document with a one-line description, grouped by
folder. Adding a document without adding its index line leaves it undiscoverable,
which defeats the point.

## Style

- Follow the global writing-style rule. No em dashes anywhere
- Write for yourself in six months with none of today's context loaded
- Record the reasoning, not just the outcome. "12V aux measured 13.8V loaded,
  so the buck needs a 15V+ input rating" is useful; "use this buck" is not
- Never put secrets in `docs/`: no WiFi PSK, no MQTT credentials, no Cloudflare
  tokens, no tunnel UUIDs
