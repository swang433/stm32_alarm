#!/usr/bin/env bash
# Build firmware.elf and firmware.bin for the Nucleo-C071RB.
# Usage: ./build.sh [source.c ...]
#   no arguments: build every .c here except the standalone tests (but.c, test.c)
#   e.g. ./build.sh
#        ./build.sh but.c
#        ./build.sh main.c irq.c oled.c
set -euo pipefail

cd "$(dirname "$0")"

if [ $# -lt 1 ]; then
    set --
    for f in *.c; do
        case "$f" in but.c|test.c) continue ;; esac
        set -- "$@" "$f"
    done
fi

# -lgcc supplies software divide (__aeabi_uidiv); the Cortex-M0+ has no hardware divider.
# It must come after the sources so the linker can resolve their references.
arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -nostdlib -Wall \
    -T linker.ld -o firmware.elf startup.s "$@" -lgcc
arm-none-eabi-objcopy -O binary firmware.elf firmware.bin
arm-none-eabi-size firmware.elf

echo "built firmware.elf and firmware.bin"