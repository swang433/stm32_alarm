# STM32 Alarm Clock — Bare-Metal Firmware

A fully functional alarm clock written in C for an ARM Cortex-M0+ microcontroller, built from scratch with **direct register access** — no HAL, no CMSIS, no standard library. Every peripheral (RTC, I2C, timers/PWM, GPIO, interrupts) is configured by hand from the reference manual, and the project ships its own startup code, vector table and linker script.

**Target:** STM32C071RB (Cortex-M0+) on a Nucleo-C071RB board

## Highlights

- **Bare-metal from reset:** custom assembly startup file, vector table and linker script; compiled with `-nostdlib`
- **Custom drivers:** SH1106 OLED over hardware I2C (400 kHz), RTC on the 32.768 kHz LSE crystal, PWM buzzer on a hardware timer
- **Interrupt-driven input:** four buttons on EXTI lines with debouncing in the ISR
- **Finite state machine:** four modes (Clock, Set Time, Set Alarm, Ring) driven by button events and RTC time
- **Alarm management:** up to 4 alarms, with duplicate and full-list rejection signalled by distinct error tones
- **Efficient display updates:** redraws only the regions that changed, with blinking driven by the RTC sub-second counter
- **Hardware debugging:** diagnosed an I2C incompatibility on a clone MCU and moved platforms (see [Engineering Challenges](#engineering-challenges))

## How It Works

The RTC keeps time from an external crystal. The main loop reads the time, compares it against the stored alarms, and updates the OLED. Button presses raise interrupts that post an event to the state machine, which changes mode or edits the time being set. When an alarm matches, the buzzer plays a melody until it is dismissed.

### State Machine

| From | Event | To |
|---|---|---|
| Clock | Set | Set Time |
| Clock | Snooze | Set Alarm |
| Clock | Alarm time reached | Ring |
| Set Time | Up / Down | Set Time (time ±1 min) |
| Set Time | Set | Clock (time written to the RTC) |
| Set Alarm | Up / Down | Set Alarm (alarm ±1 min) |
| Set Alarm | Set | Clock (alarm saved) |
| Ring | Snooze | Clock |

- **Clock:** shows the current time from the RTC
- **Set Time / Set Alarm:** Up/Down step one minute at a time (with hour carry and wraparound), and the digits blink twice a second
- **Ring:** plays the alarm melody until Snooze is pressed. Each alarm fires once per matching minute.

## Hardware

| Component | Interface | Pin(s) |
|---|---|---|
| SH1106 128×64 OLED | I2C1 | PB6 SCL (D1), PB7 SDA (D0) |
| Passive piezo buzzer | TIM3_CH2 PWM | PA7 (D11) |
| Tactile buttons ×4 | GPIO input, pull-up, EXTI | PA0 (A0), PA1 (A1), PA4 (A2), PB3 (D9) |

| Button | Pin | Action |
|---|---|---|
| Up | PA0 | In a set mode: +1 minute |
| Down | PA1 | In a set mode: −1 minute |
| Set | PB3 | In Clock: enter Set Time. In a set mode: confirm and return to Clock |
| Snooze | PA4 | In Clock: enter Set Alarm. While ringing: dismiss |

## Project Structure

```
alarm/
├── config.h     # Register definitions, enums and prototypes
├── main.c       # Peripheral init, state machine and main loop
├── irq.c        # EXTI interrupt handlers with debounce
├── rtc.c        # RTC init, set and read (LSE clock, BCD registers)
├── oled.c       # SH1106 driver over I2C and a 2× scaled digit font
├── alarm.c      # PWM buzzer driver and melodies
├── memcpy.c     # Minimal memcpy for struct copies under -nostdlib
├── startup.s    # Cortex-M0+ startup code and vector table
├── linker.ld    # Flash/RAM memory layout
└── build.sh     # Build script
```

## Building & Flashing

**Requirements:** `arm-none-eabi-gcc` toolchain and STM32CubeProgrammer

```bash
./build.sh                                        # builds firmware.elf and firmware.bin
STM32_Programmer_CLI -c port=SWD -w firmware.bin 0x08000000 -v -rst
```

The Nucleo board's onboard ST-Link handles flashing over SWD. On Windows, run `build.sh` from Git Bash.

## Engineering Challenges

### I2C incompatibility on a clone MCU
The project started on a GD32F103, a clone of the STM32F103. It is register-compatible for most peripherals, but its hardware I2C state machine behaves differently: following the standard STM32 initialization sequence, the OLED never ACKed at address `0x3C`. I worked around it by writing a bit-banged I2C driver on GPIO, which worked on the clone.

### Display throughput
The bit-banged bus ran at roughly 50 kHz. A full screen is 1,024 bytes, so full redraws were slow. Moving to the STM32C071 let me use the real I2C peripheral at 400 kHz (about 8× faster), with the hardware generating the clock and START/STOP conditions. I also changed the drawing code to update only the regions that change, never clearing the whole screen.

## Implementation Notes

- All peripherals are configured through memory-mapped register definitions in `config.h`.
- Button events reach the state machine through a single `volatile` event variable written by the ISRs.
- Debouncing uses a calibrated busy-wait loop inside the ISR (about 170 ms in practice). That is simple and reliable for this application; a timer-based debounce is a possible improvement.
- The RTC time is read from `RTC_TR` in BCD. Alarms are matched in software in the main loop, which supports multiple alarms without using the RTC's single hardware alarm.
- The Cortex-M0+ has no hardware divider, so division comes from `libgcc`.

## Roadmap

- Real snooze (re-arm the alarm a few minutes later instead of dismissing)
- Non-blocking alarm playback so buttons respond mid-melody
- Delete alarms, and save them to flash so they survive power loss
- On-screen mode labels and a cancel option in the set modes
- Battery-backed external RTC (DS3231) to keep time while unplugged

See [TODO.md](TODO.md) for the detailed task list.
