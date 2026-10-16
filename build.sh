#!/usr/bin/env bash
# Build firmware.elf and firmware.bin for the Nucleo-C071RB.
# Usage: ./build.sh <source.c> [more.c ...]
#   e.g. ./build.sh but.c
#        ./build.sh main_fsm.c irq.c oled.c
set -euo pipefail

if [ $# -lt 1 ]; then
    echo "usage: $0 <source.c> [more.c ...]" >&2
    exit 1
fi

cd "$(dirname "$0")"

# -lgcc supplies software divide (__aeabi_uidiv); the Cortex-M0+ has no hardware divider.
# It must come after the sources so the linker can resolve their references.
arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -nostdlib -Wall \
    -T linker.ld -o firmware.elf startup.s "$@" -lgcc
arm-none-eabi-objcopy -O binary firmware.elf output.bin
arm-none-eabi-size firmware.elf

echo "built firmware.elf and output.bin"