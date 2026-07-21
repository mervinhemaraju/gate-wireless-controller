---
name: log-progress
description: End-of-session wrap-up. Writes a docs/worklog entry for the work done, updates the project status and inventory, captures any decisions made, and lists what is next. Invoke with /log-progress [anything to note about the session].
allowed-tools: Read, Grep, Glob, Write(docs/**), Edit(docs/**), Edit(.claude/CLAUDE.md), Edit(.claude/inventory.yaml), Bash(git status:*), Bash(git diff:*), Bash(git log:*), Bash(ls:*), Bash(find:*), Bash(date:*)
---

# Log Progress: $ARGUMENTS

Close out the working session: write down what happened, bring the project's
state records up to date, and leave a clean starting point for next time.

`$ARGUMENTS` carries anything extra worth noting. Use it, but do not rely on
it: reconstruct the session from evidence too.

## 1. Reconstruct the Session

Do not summarise from memory alone. Gather:

- `git status` and `git diff` for uncommitted work
- `git log --oneline` since the last worklog entry, for committed work
- The conversation itself: decisions made, things learned, dead ends hit
- The most recent `docs/worklog/` entry, so this one continues from it rather
  than repeating it

Physical work (wiring, measuring, testing at the gate) leaves no git trace.
Ask the user what happened off-screen if the session involved hardware.

## 2. Write the Worklog Entry

Per `.claude/rules/docs-conventions.md`:
`docs/worklog/YYYY-MM-DD-short-slug.md`, using today's real date from `date`.
A second session on the same day gets a `-b` suffix rather than overwriting.

Cover:

- **Done:** concretely, with file or component names
- **Learned:** especially anything that contradicted an assumption. This is the
  highest-value section and the one most often skipped
- **Did not work:** failures and dead ends, kept in. A recorded dead end stops
  it being walked twice
- **Measurements:** any readings taken, with conditions and date. Cross-link to
  the `docs/wiring/` document that holds them properly
- **Next:** what the next session should pick up, and anything now blocked

Be honest about incomplete work. "Half-wired, buck converter untested" is
useful; "wiring complete" when it is not is actively dangerous on this project.

## 3. Capture Decisions Separately

If a real decision was made (a component choice, an architecture change, a
phase reordering), it gets its own `docs/decisions/NNNN-*.md` ADR with context,
options, decision and consequences. Do not bury a decision in a worklog entry:
worklogs get skimmed, ADRs get found.

## 4. Update the State Records

- **`.claude/CLAUDE.md` `## Current Status`:** rewrite it to match reality. It
  is the first thing read in a new session, so a stale status here costs more
  than anywhere else. Keep it short and current rather than appending history:
  history belongs in the worklog
- **Phase transitions:** if a phase actually completed, say so, and note what
  the next phase now needs
- **`.claude/inventory.yaml`:** if parts arrived, hand off to
  `/inventory-update` conventions rather than duplicating that logic here
- **`docs/README.md`:** add an index line for every document created this
  session

## 5. Check for Loose Ends

Before closing, flag anything left dangling:

- Hardware left partially wired, or the gate left in a non-working state
- **Mains or battery left disconnected.** Say this loudly: it means the gate is
  currently dead and the household needs to know
- Secrets accidentally written into a tracked file
- `provisional` documents still awaiting verification
- Uncommitted work the user may want to commit

## 6. Report

Summarise what was written and changed, then state the recommended starting
point for the next session in one line.

**Do not commit or stage anything.** Per the global no-git rule, list what is
worth committing and stop there.
