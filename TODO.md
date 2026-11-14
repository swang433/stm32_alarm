# TODO

## MVP

- [ ] Flash and test on the board (set an alarm 1–2 minutes ahead; an alarm for the current minute won't ring until tomorrow)

## Buttons

- [x] Call `but_init()` from `main()` with the other inits
- [x] Measure the real debounce delay: ~173 ms (13 cycles/loop at 12 MHz, ~923 loops/ms); left as is, works fine

## RTC

- [x] Split the unlock → INIT → write TR → relock part of `rtc_init()` into `rtc_set()` so set-time mode can write a new time
- [x] Remove the placeholder `rtc_set(13, 10, 20)` in `main()` (it reset the time on every boot)
- [ ] If LSE never starts (stuck on LSERDY), fall back to LSI: `RCC_CSR2` LSION, `RTCSEL = 10`, `PRER = (127 << 16) | 249`

## FSM (main.c)

- [x] Main loop: consume `button_event` → `press()` → reset to `BUTTON_NONE`
- [x] `press()` CLOCK: Set → SET_TIME (snapshot RTC into `set_hour` / `set_min`), Snooze → SET_ALARM (load last alarm)
- [x] `press()` SET_TIME / SET_ALARM: Up / Down step one minute with carry, wrapping 0–23 / 0–59; Set confirms and returns to CLOCK
- [x] SET_ALARM confirm: store in `alarms[]`, reject duplicates (`same_time`, `duplicate_alarms_sound()`) and a full array
- [x] Per-mode drawing: live RTC time in CLOCK, edited `set_hour:set_min` in the set modes (same screen area, so no clear needed)
- [x] Blink the time in the set modes: `RTC_SSR >> 6` quarters, redraw only when the quarter changes
- [x] Force a redraw after every button press (`last_s` / `last_quarter` reset to `0xFF`)
- [ ] Show the new value right after an Up/Down press (the forced redraw can land in a hidden quarter)
- [ ] Show which mode is active (SET TIME / SET ALARM label; needs more font glyphs)
- [ ] Cancel path out of SET_TIME / SET_ALARM without committing
- [ ] Rename the full-array error to its own sound (it currently plays `no_addr_sound()`)

## Alarm

- [x] In CLOCK, compare RTC time with every entry of `alarms[]` (`same_time`, hour + minute) → enter `MODE_RING`
- [x] Ring once per matching minute (`rung` flag), so a dismiss doesn't re-trigger it
- [x] Play `wakey_wakey()` from the main loop while in `MODE_RING`; Snooze button dismisses
- [ ] Real snooze: stop sound, push the alarm a few minutes later (currently it only dismisses)
- [ ] Delete / clear alarms (once `alarm_count == MAX_ALARMS` no new alarm can be saved)
- [ ] Make the ring non-blocking, so Snooze responds mid-tune (`wakey_wakey()` blocks for the whole tune)
- [ ] Save alarms to flash (last page, write only on confirm — ~10k erase cycles)
- [ ] Decide whether `alarmset_hour` / `alarmset_min` stay once `alarms[]` is the source of truth

## Cleanup

- [x] Undo the mangled rename (`M` → `min`, `H` → `hour`): `EXTI_MASK`, `GPIOx_MODER`, `DEBOUNCE_MS`, `TIMER_TICK_HZ`, `CONFIG_H`, `fsm_mode`, `current_mode`
- [x] Remove `extern struct Time current_time` in `irq.c` (unused; `Time` itself is now used for `alarms[]`)
- [x] Remove unused `SIZE_TIME`, `alarm_sound` and `time_changed`
- [x] Convert FSM mode and button `#define`s to enums (`fsm_mode_t`, `button_t`) and the variables/parameters that hold them
- [x] `build.sh` writes `firmware.bin`, matching the README
- [x] `build.sh`: with no args, builds every `.c` except `but.c` and `test.c` (both are standalone tests)
- [x] Whole-struct copies: solved by providing `memcpy` in `memcpy.c`
- [ ] Stale comments: "field-by-field" above `alarms[alarm_count] = new_alarm;` and "placeholder start time" next to `rtc_init()` in `main.c`; "and set the time" above `rtc_init()` in `rtc.c`

## Nice to have

- [ ] HardFault handler: start a tone, then show the faulting PC in hex on the OLED
- [ ] Battery-backed DS3231 on the shared I2C bus (`0x68`) so time survives unplugging
- [ ] Date display (read `RTC_DR`, small font on a lower page)
