# Worklog - 2026-07-29

- **Date:** 2026-07-29
- **Phase:** 2

## Done

- Mounting-box dry-layout check and GPIO pin assignment (Stage A close-out):
  ESP32 + breakout, relay, and buck converter confirmed to fit the IP67 box;
  decided to mount the box open rather than sealed, relying on the D3 motor
  housing's own IP55 rating instead. Recorded in
  `docs/wiring/03-mounting-layout.md`,
  `docs/decisions/0003-open-mounting-box.md`, and
  `docs/wiring/04-gpio-pin-map.md` (`RELAY_TRIGGER_PIN` = GPIO27,
  `STATUS_SENSE_PIN` = GPIO34). This closed out every bench-decidable Stage A
  item.
- Amended `.claude/skills/mqtt-contract/SKILL.md`: `gate/cmd/ack`'s `result`
  field is now an enumerated closed set (`accepted`, `duplicate`,
  `rejected_cooldown`, `rejected_busy`, `rejected_malformed`), not just a
  loose example. Needed because the firmware skeleton introduces these
  values and the contract's own rule requires them written down before
  the server/app are built against them.
- Built the first firmware code in the repo: `firmware/` skeleton, planned
  and approved via plan mode, then implemented in full:
  - `lib/gate_control/` - the safety-critical trigger/cooldown/dedup state
    machine. Request-id dedup is tracked independently of the cooldown
    timer (a duplicate is rejected even long after cooldown has elapsed,
    since MQTT QoS 1 redelivery can arrive late). Boot-safety sequence
    (`gateControlHardwareInit()`) latches the relay idle before `pinMode`
    is ever called on the pin.
  - `lib/status_decoder/` - fully config-agnostic flash-pattern decoder.
    Takes zero dependency on `config.h`; all timing and polarity come in
    via `StatusDecoderConfig`, so it's testable with fixture values
    unrelated to production numbers.
  - `lib/wifi_manager/`, `lib/mqtt_client/` - non-blocking connect/reconnect
    with exponential backoff and jitter; MQTT client implements the LWT,
    retained/non-retained publishes, and schema-version checking exactly
    per the contract.
  - `src/main.cpp`, `include/config.h`, `include/logging.h`,
    `include/secrets.h.example` - thin entry point, named-constants-only
    config, leveled logging, secrets template.
  - `test/test_gate_control/`, `test/test_status_decoder/` - 24 native
    unit tests, no hardware required.
  - `firmware/README.md` - build/test/upload instructions, secrets setup,
    and the Phase 3 placeholder list.
- Installed PlatformIO (`pip3 install -U platformio`) and ran real
  verification rather than trusting the plan on paper:
  - `pio test -e native`: 24/24 passed
  - `pio run -e esp32dev`: compiles clean (RAM 15.1%, Flash 62.4%)
- Updated `.gitignore`: `.pio/` and `firmware/include/secrets.h`.

## Learned

- **`unsigned long` is not a portable stand-in for `millis()`'s 32-bit
  width.** On this native/desktop build `unsigned long` is likely 64-bit,
  while the real ESP32 target's `millis()` is a 32-bit value that wraps
  after ~49.7 days. Caught this before writing any tests and switched every
  millis()-domain time value (in `gate_control`, `status_decoder`,
  `wifi_manager`, `mqtt_client`) to explicit `uint32_t`, so the rollover
  tests actually exercise the same overflow behaviour on `native` as on
  hardware, rather than two different things that happen to share a type
  name.
- **PlatformIO does not reliably auto-add `include/` to the compiler path
  for `lib/` sources under the `native` test runner**, contrary to what the
  approved plan assumed. Only discovered by actually running `pio test`;
  fixed with an explicit `-Iinclude` build flag shared by both environments
  in `platformio.ini`. Worth remembering for any future PlatformIO project
  in this repo: verify this rather than assume it.
- A test helper bug (see Did Not Work) reinforced that test infrastructure
  needs the same scrutiny as production code - it can hide a real defect or
  fabricate a false pass just as easily.

## Did Not Work

- First `status_decoder` test run: 2 of 12 tests failed
  (`test_period_matching_fault_band_is_fault`,
  `test_thresholds_are_data_driven_not_hardcoded`). Root cause was in the
  test helper (`feedOnePeriod`), not the production code: it added a raw
  level change every `periodMs`, but each edge is only *accepted* after an
  additional `debounceMs`, so the actually-measured period between accepted
  edges was `periodMs + debounceMs`, not `periodMs`. Wide bands (opening/
  closing) absorbed the 20ms error and passed by coincidence; the
  tight-tolerance fault-band test did not. Fixed by timing the second raw
  flip at `periodMs - debounceMs` after the first accepted edge instead.
- First `pio run -e esp32dev` attempt failed on `secrets.h` not existing -
  expected and by design (gitignored, developer-provided). Verified the
  rest of the build by copying `secrets.h.example` to `secrets.h` locally
  (placeholder values, still gitignored) and rebuilding; left that copy in
  place since it matches the README's own documented setup step.

## Measurements

None this session - Stage A's bench measurements (relay polarity, mounting
fit) were finished and recorded in the previous entries; today was pin
assignment, decision-recording, and software.

## Next

- Stage B continues: FastAPI skeleton + Mosquitto config on the Pi side,
  Cloudflare Tunnel + Zero Trust Access policy setup. Both have no gate or
  firmware dependency and can proceed now.
- `GateState::STOPPED` is defined in the enum but the current
  `status_decoder` classifier never produces it - noted explicitly in
  `firmware/README.md` as a known gap, not forgotten.
- Same loose end as the last two entries, still unaddressed: bench photos
  from Stage A live in `images/` outside `docs/wiring/img/` (except the one
  mounting-layout photo already moved). Lower priority now that Stage A is
  closed out.
- Before any real CP80 wiring: bench-test the full firmware with an LED
  standing in for the relay first, per `firmware/README.md` and hard
  condition 5. Not done this session - entirely desk/software work today.
