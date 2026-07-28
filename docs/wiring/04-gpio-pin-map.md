# GPIO Pin Map

- **Date:** 2026-07-29
- **Phase:** 2 (assignment desk work; electrical polarity on the status pin
  is provisional, see Open Items)
- **Status:** verified for pin selection and exclusions. `STATUS_SENSE_PIN`'s
  active-high/active-low meaning is provisional pending Phase 3.

Board: 30-pin ESP32 (CP2102 TYPE-C) on the matching screw-terminal breakout.
Constraints below per `.claude/skills/esp32-firmware/SKILL.md`.

## Assignments

| Constant | GPIO | Direction | Function | Idle level | Active level |
|---|---|---|---|---|---|
| `RELAY_TRIGGER_PIN` | 27 | Output | Pulses the HW-803 relay, which pulses CP80 `TRG`-`COM` | LOW | HIGH |
| `STATUS_SENSE_PIN` | 34 | Input (input-only, ADC1) | Reads the CP80 `LED` terminal through the resistor divider | Not yet determined [UNVERIFIED, Phase 3] | Not yet determined [UNVERIFIED, Phase 3] |

## Why These Two Pins

**`RELAY_TRIGGER_PIN` = GPIO27.** Excluded, per the firmware skill's GPIO
rules:

- Strapping pins (0, 2, 5, 12, 15) - a line that must be quiet at boot,
  which this is, must avoid these entirely
- Flash pins (6-11) - not broken out on this breakout board anyway
- Input-only pins (34-39) - cannot drive an output at all
- UART0 (GPIO1/3) - avoided so the serial console/flashing stays usable and
  never has boot-time chatter near the trigger line

GPIO27 is a plain bidirectional GPIO with no boot-time role, and is broken
out as `D27` on this breakout board [visual, board photographed 2026-07-29].

**`STATUS_SENSE_PIN` = GPIO34.** The firmware skill excludes GPIO34-39 for
"the pullup-dependent optocoupler input", because they have no internal
pull-up and the PC817 design needed one (`INPUT_PULLUP`, active-low). That
exclusion no longer applies: the PC817 was rejected
([ADR 0002](../decisions/0002-resistor-divider-over-optocoupler.md)) in
favour of a resistor divider, which actively drives the pin to a real
voltage rather than needing a pull-up to sense an open-collector output.
GPIO34 being input-only is therefore not a problem here, and being an ADC1
channel is a free bonus - firmware can read it as a plain digital HIGH/LOW
or sample it as analog voltage if Phase 3 calibration wants the raw level
rather than just a threshold.

## Deviations From the Skill's Generic Defaults

Recorded explicitly so `config.h` is written against this project's
measured facts, not the skill's generic assumptions:

- Skill assumes "relay modules in this build are opto-isolated and
  typically active-low, so idle is usually HIGH". **Measured result for
  this specific board is the opposite: active-HIGH, idle LOW** - see
  [02-relay-module.md](02-relay-module.md). `config.h` must encode
  active-HIGH, not the skill's default
- Skill assumes a PC817 status input ("pulls the GPIO low when the status
  LED is lit... treat the signal as active-low"). **This project uses a
  resistor divider instead**, per
  [ADR 0002](../decisions/0002-resistor-divider-over-optocoupler.md). No
  `INPUT_PULLUP` is needed, and the active-high/active-low meaning is not
  yet known - it depends on the CP80 `LED` terminal's behaviour, measured
  under load in Phase 3, not on an assumption carried over from the PC817
  design

## Open Items

- `STATUS_SENSE_PIN`'s active/idle voltage meaning: waits on the CP80 `LED`
  terminal being measured under load (Phase 3), per ADR 0002. Do not encode
  a guessed polarity in firmware; keep it as a named, clearly-marked
  placeholder until that measurement exists
- No pin is yet assigned for a Phase 5 pedestrian-mode input - out of phase,
  not needed now, and would need its own relay module regardless (see
  [02-relay-module.md](02-relay-module.md))
- This is a design record, not code. `firmware/` does not exist yet;
  whichever session starts the firmware skeleton should encode this table
  directly into `config.h` as named constants, per the firmware skill's
  structure rules
