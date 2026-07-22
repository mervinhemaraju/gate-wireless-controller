---
name: whats-next
description: Assess the current state of the project and list every piece of work that could be done right now, grouped by whether it is unblocked, blocked, or out of phase. Invoke with /whats-next [optional focus area].
allowed-tools: Read, Grep, Glob, Bash(git log:*), Bash(git status:*), Bash(ls:*), Bash(find:*)
---

# What's Next: $ARGUMENTS

Work out where the project actually stands and lay out everything that could be
done from here. **Assess and report only: change nothing.**

If `$ARGUMENTS` names a focus area (a phase, "firmware", "docs"), still assess
the whole project but lead with that area.

## 1. Establish Real State

Never answer from memory or from the status section alone: it goes stale. Read
the evidence first:

- `.claude/CLAUDE.md`: the declared phase and status
- `.claude/inventory.yaml`: what has actually arrived versus what is on order
- `docs/`: the worklog's most recent entries, and any `provisional` documents
  awaiting verification
- Repo layout: which of `firmware/`, `server/`, `app/` exist and contain code
- `git log --oneline -15`: what has actually been happening

Where the declared status and the evidence disagree, trust the evidence and
**say so explicitly**. A stale status line is itself a finding.

## 2. Respect Phase Discipline

`CLAUDE.md` mandates strict phases. Honour that ordering:

- Phase 5 items stay out of the actionable list until Phases 1-4 work, no
  matter how appealing. List them under "out of phase" so they are captured
  but not started
- Firmware and server work is Phase 2 and proceeds now against the MQTT
  contract: the MQTT contract itself, the status decoder state machine and its
  native tests, the FastAPI skeleton and the Access policy design all proceed
  on a desk. Two items cannot be finalised until Phase 3 hardware exists: the
  LED flash-code to gate-state mapping and the final GPIO pin assignments
- Physical wiring, hardware measurement and end-to-end integration are Phase 3
  and need parts in hand plus mains and battery isolated

## 3. Lead With Phase Status

Open the report with where the current phase actually stands:

```markdown
### Phase <N> - <Name> - <In Progress | Complete | Blocked>

**Done**
- confirmed completed steps

**Pending**
- what still has to happen before this phase closes

**Blockers**
| Blocker | Type | Needed to unblock |
|---|---|---|
| ... | Parts / Hardware / Software / Decision | ... |

**Deliverable met?** Yes / No, and one sentence on why.
```

The deliverable comes from the `## Phases` section of `CLAUDE.md`. Judge it
against evidence, not against the status line: "mostly done" is not met.

## 4. Group the Work

Present findings in exactly these groups:

**Unblocked now.** Everything genuinely startable today. For each: what it is,
which phase, roughly how long, and what it unblocks downstream.

**Blocked.** For each: what it is waiting on and who or what clears the block.
Distinguish a delivery wait (parts in transit) from a decision wait (needs an
answer) from a physical wait (needs to be at the gate, needs daylight, needs
power isolated).

**Needs a decision from you.** Open questions where the project cannot proceed
until a choice is made. State the options and give a recommendation.

**Out of phase.** Phase 5 and anything else deliberately deferred, so it is
recorded rather than forgotten.

## 5. Recommend

Close with a single recommended next action and one sentence of reasoning.
Prefer whatever removes the most downstream risk: verifying an assumption that
much else depends on usually beats making visible progress on something safe.

Call out anything time-sensitive: a part that needs reordering, a measurement
only possible while the gate is already opened up, an unverified assumption
that gets more expensive to fix the longer it stands.

## 6. Do Not

- Do not start any of the work. This skill reports, it does not implement
- Do not propose wiring specifics here. That is
  `.claude/rules/wiring-research.md` territory and needs sourced verification
- Do not commit or stage anything
