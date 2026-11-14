# STM32 Alarm Clock

Bare-metal alarm clock firmware written in C with direct register access — no HAL or CMSIS abstraction layer.

> **Prototyping board:** Nucleo-C071RB (STM32C071RB, Cortex-M0+). Config registers are in `config.h`.

## Features

- Real-time clock using the STM32 internal RTC peripheral
- SH1106 OLED display over I2C
- Passive piezo buzzer driven by PWM
- Four buttons with software busy-wait debouncing
- Finite state machine with four operating modes
- Up to 4 alarms, with duplicate detection

## Hardware

| Peripheral | Interface | Pin(s) |
|---|---|---|
| SH1106 OLED Display | I2C | PB6 SCL (D1), PB7 SDA (D0) |
| Passive Piezo Buzzer | PWM | PA7 (D11) |
| Tactile Buttons (×4) | Digital Input (pull-up) | PA0 (A0), PA1 (A1), PA4 (A2), PB3 (D9) |

**Target MCU:** STM32C071RB on Nucleo-C071RB (previously STM32F103)

## Button Map

| MCU Pin | Arduino Header | Button | Action |
|---|---|---|---|
| PA0 | A0 | Up | In a set mode: one minute later (carries into the hour) |
| PA1 | A1 | Down | In a set mode: one minute earlier |
| PB3 | D9 | Set | In clock mode: enter set time. In a set mode: confirm and return to clock |
| PA4 | A2 | Snooze | In clock mode: enter set alarm. While ringing: dismiss the alarm |

## FSM Modes

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

- **Clock**: default display, shows the current time from the RTC
- **Set Time**: up/down change the time one minute at a time; set writes it to the RTC
- **Set Alarm**: up/down change the alarm time; set saves it (rejects duplicates and a full list)
- **Ring**: plays the alarm tune until snooze is pressed; each alarm rings once per matching minute

The time blinks twice a second in both set modes.

## Project Structure

```
alarm/
├── config.h       # Register definitions, enums and prototypes for STM32C071RB
├── main.c         # Button setup, FSM logic and main loop
├── irq.c          # EXTI interrupt handlers with software debounce
├── rtc.c          # RTC on the 32.768 kHz LSE: init, set and read the time
├── oled.c         # SH1106 display driver (I2C) and 2x digit font
├── alarm.c        # Buzzer driver (TIM3_CH2 PWM on PA7) and melodies
├── memcpy.c       # memcpy for struct copies, since the build uses -nostdlib
├── startup.s      # Cortex-M0+ startup and vector table in assembly
├── linker.ld      # Linker script for flash/RAM layout
├── build.sh       # Builds firmware.elf and firmware.bin
└── TODO.md        # Remaining work
```

## Building & Flashing

Requires `arm-none-eabi-gcc`, `arm-none-eabi-objcopy`, and STM32CubeProgrammer (for flashing). The startup file (`startup.s`) and linker script (`linker.ld`) are included.

**1. Build** compiles, links and converts to binary in one step. Run it in Git Bash (not WSL, and not `.uild.sh` from PowerShell). With no arguments it builds every `.c` file except the standalone tests `but.c` and `test.c`; pass files to build only those (`startup.s` and `linker.ld` are added automatically):
```bash
./build.sh
./build.sh but.c
```

This produces `firmware.elf` and `firmware.bin`.

**2. Flash via STM32CubeProgrammer (ST-Link)** — the Nucleo-C071RB has an onboard ST-Link; no bootloader pin toggling needed.

*GUI:* open STM32CubeProgrammer, select **ST-LINK** as the interface, click **Connect**, open `firmware.bin` (or `firmware.elf`) in the **Erasing & Programming** tab, set the start address to `0x08000000` for a `.bin`, and click **Start Programming**.

*CLI:*
```bash
STM32_Programmer_CLI -c port=SWD -w firmware.bin 0x08000000 -v -rst
```

`-v` verifies after writing and `-rst` resets the board so the new firmware starts running.

## Challenges Faced

### GD32F103 I2C Peripheral Incompatibility
The target board was a GD32F103C8T6, a Chinese STM32F103-compatible clone. While the GD32 is pin and register-compatible with the STM32 for most peripherals, the hardware I2C peripheral behaves differently — the GD32's I2C state machine has timing differences that cause it to silently fail when following the standard STM32F103 register-level initialization sequence. The OLED would not be detected at address `0x3C` and no ACK was received. The fix was to abandon the hardware I2C peripheral entirely and implement bit-bang I2C on PB6/PB7 using direct GPIO toggling, which bypasses the peripheral and works identically on both chips. This was also later abandoned in favor of the STM32C071RB Nucleo Board which contains the actual STM32C071 chip that contains proper I2C peripherals.

### Display Refresh Speed Bottleneck
On the GD32, the bit-bang I2C implementation ran at approximately 50kHz due to the 50-NOP `dly()` calls inserted between every SCL/SDA transition. Clearing the full display requires 1024 I2C bytes (128 columns × 8 pages), each requiring 9 clock pulses with multiple delay calls. This made a full clear + redraw take significant time, limiting how fast the display could be updated.

This was resolved by moving to the Nucleo-C071RB, where the hardware I2C1 peripheral is used instead (`TIMINGR` configured for a 400kHz bus, roughly 8× the old bit rate, with the hardware handling the clock and START/STOP). To keep updates fast, the display code should still avoid full clears and only redraw the regions that changed.

## Implementation Notes

- All peripherals are configured by direct memory-mapped register writes defined in `config.h`.
- Button debounce is handled inside each EXTI ISR using a busy-wait NOP loop (`DEBOUNCE_MS = 20`, which comes out to around 170 ms of actual debouncing at 12 MHz).
- Button events are communicated to the FSM via a `volatile` global flag (`button_event`).
- The RTC runs from the LSE crystal; the time is read from `RTC_TR` (BCD) and blinking uses the sub-second counter `RTC_SSR`. Alarms are matched in the main loop rather than with the RTC alarm hardware.