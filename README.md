# STM32 Alarm Clock

Bare-metal alarm clock firmware written in C with direct register access — no HAL or CMSIS abstraction layer.

> **Prototyping board:** Nucleo-C071RB (STM32C071RB, Cortex-M0+). Config registers are in `config_c071.h`. The original STM32F1xx register definitions are preserved in `config_stm32f.h`.

## Features

- Real-time clock using the STM32 internal RTC peripheral
- SSH1106 OLED display over I2C
- Passive piezo buzzer driven by PWM
- Four-button input with hardware debouncing via EXTI interrupts
- Finite state machine with three operating modes

## Hardware

| Peripheral | Interface | Pin(s) |
|---|---|---|
| SSH1106 OLED Display | I2C | PB6 (SCL), PB7 (SDA) |
| Passive Piezo Buzzer | PWM | PA6 |
| Tactile Buttons (×4) | Digital Input (pull-up) | PA0–PA3 |

**Target MCU:** STM32C071RB on Nucleo-C071RB (previously STM32F103)

## Button Map

| Pin | Button | Action |
|---|---|---|
| PA0 | Increment | Increases the selected time field |
| PA1 | Decrement | Decreases the selected time field |
| PA2 | Alarm Set | Enters/confirms alarm setting mode |
| PA3 | Snooze | Snoozes an active alarm |

## FSM Modes

```
MODE_CLOCK (0) ──► MODE_SET_TIME (1)
     ▲                    │
     └──── MODE_SET_ALARM (2) ◄──┘
```

- **Clock** — default display, shows current time
- **Set Time** — adjust hours and minutes using increment/decrement
- **Set Alarm** — configure the alarm trigger time

## Project Structure

```
alarm/
├── config.h       # Memory-mapped register definitions and macro constants
├── irq.c          # EXTI interrupt handlers with software debounce
├── main_fsm.c     # FSM logic and main entry point
├── oled.c         # SSH1106 display driver (I2C)
├── startup.s      # (planned) Cortex-M3 startup and vector table in assembly
└── linker.ld      # (planned) Linker script for flash/RAM layout
```

## Building & Flashing

Requires `arm-none-eabi-gcc`, `arm-none-eabi-objcopy`, and `stm32flash`. The startup assembly file and linker script are planned (see file tree below).

**1. Compile and link**
```bash
arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -nostdlib -T linker.ld -o firmware.elf startup.s main.c
```

**2. Convert to binary**
```bash
arm-none-eabi-objcopy -O binary firmware.elf firmware.bin
```

**3. Flash via ST-Link** — the Nucleo-C071RB has an onboard ST-Link; no bootloader pin toggling needed.
```bash
st-flash write firmware.bin 0x08000000
```

## Challenges Faced

### GD32F103 I2C Peripheral Incompatibility
The target board uses a GD32F103C8T6, a Chinese STM32F103-compatible clone. While the GD32 is pin and register-compatible with the STM32 for most peripherals, the hardware I2C peripheral behaves differently — the GD32's I2C state machine has timing differences that cause it to silently fail when following the standard STM32F103 register-level initialization sequence. The OLED would not be detected at address `0x3C` and no ACK was received. The fix was to abandon the hardware I2C peripheral entirely and implement bit-bang I2C on PB6/PB7 using direct GPIO toggling, which bypasses the peripheral and works identically on both chips.

### Display Refresh Speed Bottleneck
The bit-bang I2C implementation runs at approximately 50kHz due to the 50-NOP `dly()` calls inserted between every SCL/SDA transition. Clearing the full display requires 1024 I2C bytes (128 columns × 8 pages), each requiring 9 clock pulses with multiple delay calls. This makes a full `oled_clear()` + redraw take significant time, limiting how fast the display can be updated. Options to address this: switch to the hardware I2C peripheral at 400kHz (blocked by the GD32 compatibility issue above), reduce NOP count in `dly()` (risks missed ACKs), or avoid full clears by only redrawing changed regions.

## Implementation Notes

- All peripherals are configured by direct memory-mapped register writes defined in `config.h`.
- Button debounce is handled inside each EXTI ISR using a busy-wait NOP loop (`DEBOUNCE_MS = 20 ms`).
- Button events are communicated to the FSM via a `volatile` global flag (`button_event`).
- The RTC uses the STM32's internal `RTC_CNT` counter with alarm support via `RTC_ALR`.
