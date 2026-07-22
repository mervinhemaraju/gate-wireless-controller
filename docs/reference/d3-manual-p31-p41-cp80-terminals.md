# CP80 Terminal Identification (D3/D5 Manual)

- **Date:** 2026-07-22
- **Phase:** 3 reference (feeds Phase 2 firmware design and Phase 3 wiring)
- **Status:** verified (faithful transcription of manual pp.31, 40, 41). Values
  applied to this specific gate still need on-gate measurement where noted.

Source: Centurion D3/D5 installation manual, read visually from the scanned PDF
(poppler render, 2026-07-22). No tesseract; transcription is high-confidence but
confirm any doubtful value against the PDF page directly.

## Signal terminal row, left to right [manual p.40, p.41]

```
LIGHT  COM  COM | 12V  TRG  IRB  FRX  LED  PED  LCK  SET
```

The power row (separate, to the left) is: `MOTOR  - BATTERY +  LIGHT  COM  COM`.

## Terminal functions [manual p.31, "Controller Terminal Features"]

- **COM** - "The battery/power supply negative terminal. All trigger signals
  etc. have their return path to one of the COM terminals." So COM is the
  system negative / common ground. [manual p.31]
- **LIGHT** - two terminals, a normally-open potential-free contact, normally
  used to switch a pillar/courtesy light. [manual p.31]
- **12V** - "Provides a +12V DC supply for auxiliary equipment such as a radio
  receiver, photo cells etc. It is linked directly to the battery via the 3A
  fast-blow auxiliary fuse (see page 41)." This is the rail intended to power
  the ESP32 via the buck converter. See `d3-manual-p10-specifications.md` for
  the system voltage and `AUXILIARY FUSE 3A F/B` on p.40. [manual p.31, p.40]
- **TRG** - "A momentary, normally-open trigger device such as relay,
  pushbutton etc. connected between TRG and COM will cause the gate to trigger
  open/closed. Connect multiple trigger devices in parallel." This directly
  confirms the project's trigger method: a relay contact pulsed across TRG-COM.
  [manual p.31, p.41]
- **IRB** - infra red beam / safety input. Normally-closed contact between IRB
  and COM. NB from the manual: "If no safety devices are fitted ensure a wire
  link is fitted between IRB and COM." [manual p.31]
- **FRX** - free-exit input. Momentary normally-open contact, opens or re-opens
  a closing gate; never initiates a closing cycle. [manual p.31]
- **LED** - "An output terminal which provides a low current, (approx. 4,5V DC,
  20mA) to drive an LED which can be used to indicate the gate status remotely.
  If more than 3 LED's are required it is necessary to fit a multi LED driver
  card (CP78)." Decimal is a comma in the source (4,5V = 4.5V). [manual p.31]
- **PED** - pedestrian keyswitch input (normally-open, return spring). [p.41]
- **LCK** - holiday lockout keyswitch input (normally-closed). [p.41]
- **SET** - programming set terminal. [p.41]

## Key consequence for the status-sense design

The dedicated **LED** terminal is a wired status output at approx **4.5V DC,
20mA** [manual p.31], and it follows the same status pattern as the onboard
STATUS LED (flash codes in `d3-manual-p45-led-indicator-lights.md`). This is a
cleaner status source than optically reading the onboard STATUS LED.

This contradicts the current inventory note, which describes reading the status
LED as a "12V signal". The LED terminal is ~4.5V DC / 20mA, not 12V. Measure it
in both gate states before sizing the optocoupler front end. [UNVERIFIED on this
gate: measure]

## Board

Silkscreen reads "CENTURION SYSTEMS CP80 V1.2". Fuses on the board: MASTER FUSE
10A S/B, AUXILIARY FUSE 3A F/B, LIGHT FUSE 3A fast blow. [manual p.40]
