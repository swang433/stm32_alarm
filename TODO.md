# TODO

## Buttons
- [x] Call `but_init()` from `main()` with the other inits
- [x] Measure the real debounce delay: ~173 ms (13 cycles/loop at 12 MHz, ~923 loops/ms); left as is, works fine

## RTC
- [x] Split the unlock → INIT → write TR → relock part of `rtc_init()` into `rtc_set()` so set-time mode can write a new time
- [ ] If LSE never starts (stuck on LSERDY), fall back to LSI: `RCC_CSR2` LSION, `RTCSEL = 10`, `PRER = (127 << 16) | 249`

## FSM (main.c)
- [ ] Main loop: consume `button_event` → `press()` → reset to `BUTTON_NONE`
- [ ] `press()`: Alarm Set cycles CLOCK → SET_TIME (h → m) → SET_ALARM (h → m) → CLOCK
  - [ ] CLOCK → SET_TIME: snapshot RTC into `edit_h` / `edit_m`
  - [ ] SET_TIME → SET_ALARM: write edited time with `rtc_set()`
  - [ ] Increment / Decrement change the selected field, wrapping 0–23 / 0–59
- [ ] Per-mode drawing: live RTC time in CLOCK, edited values in the set modes
- [ ] Show which mode / field is active (blinking field or a label; needs more font glyphs)

## Alarm
- [ ] In CLOCK, compare RTC time with `alarm_h:alarm_m` (and `s == 0`) → set `alarm_sound`, start buzzer
- [ ] Snooze: stop sound, push alarm a few minutes later
- [ ] Save alarm time to flash (last page, write only on confirm — ~10k erase cycles)

## Cleanup
- [ ] Remove `struct Curr_Time` (`config.h`) and `extern struct Curr_Time current_time` (`irq.c`); the RTC is the source of truth
- [ ] Convert FSM mode and button `#define`s to enums (and the variables/parameters that hold them)
- [ ] `build.sh` writes `output.bin` but the README says `firmware.bin`; make them match
- [ ] `build.sh`: default to building every `.c` except `but.c` when no args are given

## Nice to have
- [ ] HardFault handler: start a tone, then show the faulting PC in hex on the OLED
- [ ] Battery-backed DS3231 on the shared I2C bus (`0x68`) so time survives unplugging
- [ ] Date display (read `RTC_DR`, small font on a lower page)
