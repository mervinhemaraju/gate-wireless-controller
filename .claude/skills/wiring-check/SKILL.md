---
name: wiring-check
description: Read-only pre-flight safety checklist before any Phase 3 wiring session on the D3 motor. Run before touching the CP80 board. Invoke with /wiring-check [what you are about to wire].
allowed-tools: Read
---

# Wiring Pre-Flight: $ARGUMENTS

Run this before any work inside the motor housing. It is a checklist, not a
task: **make no edits and run no commands.** Walk the user through it and wait
for an answer at each gate.

The hard conditions in `.claude/CLAUDE.md` are passive context the rest of the
time. This turns them into an active check at the one moment they matter.

## 1. Power Isolation

Ask, and wait for explicit confirmation of each:

- [ ] Mains supply to the D3 switched off at the isolator
- [ ] **Battery disconnected.** The D3 is battery-backed, so killing the mains
      leaves the board fully live. This is the step most likely to be skipped
      and the one that matters most
- [ ] Absence of voltage confirmed with the multimeter, not assumed from the
      switch position

If any answer is no, stop here. Do not continue the checklist and do not
discuss wiring steps until it is yes.

## 2. Design Check

Confirm the planned work does not violate a hard condition:

- [ ] Nothing being run between the Pi and the gate. The Pi is never wired to
      the motor; that link is wireless only (hard condition 1)
- [ ] Every connection is on the 12V low-voltage side of the CP80 (condition 2)
- [ ] Nothing being changed that affects the NOVA remote receiver. The existing
      remotes must keep working untouched (condition 4)

## 3. Measure Before Connecting

- [ ] 12V aux output measured under load, with the actual reading noted. It is
      a nominal 12V that sags and spikes; the buck converter input range has to
      cover what is really there
- [ ] Status LED output measured in both states, so the PC817 input resistor
      and polarity are chosen against a real number
- [ ] Multimeter readings recorded in `docs/`, not just remembered

## 4. Bench Test Before Gate

- [ ] Firmware bench-tested with an LED standing in for the relay
- [ ] Relay polarity confirmed (these modules are commonly active-low, so idle
      is usually HIGH). Verified on the bench, not assumed from the silkscreen
- [ ] Boot behaviour watched at least twice: the relay must not click on power
      up. A boot-time glitch on TRG is an unattended gate opening
- [ ] Trigger pulse width confirmed bounded in firmware

## 5. Mechanical

- [ ] Dry layout of the mounting box inside the housing before drilling
      anything. Available space is roughly 10x8cm floor, 8cm depth
- [ ] Nothing left hanging by its wires, nothing touching a moving part or the
      motor's heat path
- [ ] Wire routing clear of the gate mechanism and strain-relieved

## 6. Before Re-Energising

- [ ] All connections re-checked against the plan
- [ ] No stray strands outside a ferrule, no tools left in the housing
- [ ] Reconnect mains first, battery second, and watch for the relay clicking
      on the D3's own power up

## Report

Summarise which items passed, which were skipped, and which blocked. If
anything blocked, state plainly that wiring should not proceed.
