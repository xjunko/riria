#!/bin/env bash
set -euo pipefail

if [ $# -lt 1 ]; then
    echo "Usage: $0 <file.c>"
    exit 1
fi

SRC="$1"
BASENAME=$(basename "$SRC" .c)

export PATH="/home/lain/Projects/Tooling/Cross/x86_64/gcc-x86_64-elf_15.2.0/bin/:$PATH"

CC=x86_64-elf-gcc
OBJCOPY=x86_64-elf-objcopy

$CC -O0 -ffreestanding -fno-pie -fno-pic \
    -nostdlib -fno-stack-protector \
    -static \
    -I./libc/include \
    "$SRC" libc/libriria.a \
    -T link.ld \
    -o "$BASENAME.elf"

$OBJCOPY -O binary "$BASENAME.elf" "$BASENAME.bin"

mkdir -p ../initramfs
mv "$BASENAME.bin" ../initramfs/
mv "$BASENAME.elf" ../initramfs/

