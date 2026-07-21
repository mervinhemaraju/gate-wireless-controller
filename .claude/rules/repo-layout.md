# Repository Layout: Keep CLAUDE.md in Sync

Whenever a new root-level folder is added to this repository, add it to the
`## Planned Repository Layout` list in `.claude/CLAUDE.md` before finishing the
task. That list is how a new session learns the shape of the repo, so a folder
missing from it is effectively invisible.

## What to Add

A bullet in the existing format, keeping the list in the order things get
built rather than alphabetical:

```markdown
- `folder-name/` - one line on what it is and which phase it belongs to
```

## When This Applies

- You create a new root-level folder
- The user says they have added, or are about to add, a root-level folder
- You notice a root-level folder with no entry in the list

## What Counts as Root-Level

Direct children of the repo root only: `firmware/`, `server/`, `app/`, `docs/`.
Subfolders do not get their own entry, with one exception: `.claude/rules/` and
`.claude/skills/` are listed, because they change how every future session
behaves.

## Related

Structure *inside* `docs/` is governed separately by
`.claude/rules/docs-conventions.md`. This rule covers the repo root only.
