# STM32 Alarm Clock

Bare-metal alarm clock firmware for the STM32F1xx, written in C with direct register access — no HAL or CMSIS abstraction layer.

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

**Target MCU:** STM32F103 (or compatible STM32F1xx)

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
arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -nostdlib -T linker.ld -o firmware.elf startup.s main.c
```

**2. Convert to binary**
```bash
arm-none-eabi-objcopy -O binary firmware.elf firmware.bin
```

**3. Enter bootloader** — set Boot0 pin high, then reset the board.

**4. Flash over UART from linux**
```bash
stm32flash -w firmware.bin -v -g 0x0 [UART_DEVICE_NAME]
```

**5. Boot normally** — set Boot0 pin low, then reset the board.

## Implementation Notes

- All peripherals are configured by direct memory-mapped register writes defined in `config.h`.
- Button debounce is handled inside each EXTI ISR using a busy-wait NOP loop (`DEBOUNCE_MS = 20 ms`).
- Button events are communicated to the FSM via a `volatile` global flag (`button_event`).
- The RTC uses the STM32's internal `RTC_CNT` counter with alarm support via `RTC_ALR`.
