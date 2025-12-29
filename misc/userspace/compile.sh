#!/bin/env bash
set -euo pipefail

if [ $# -lt 1 ]; then
    echo "Usage: $0 <file.c>"
    exit 1
fi

SRC="$1"
BASENAME=$(basename "$SRC" .c)

export PATH="/home/lain/Projects/OSDev/Porting/toolchain/usr/bin/:$PATH"

CC=x86_64-riria-gcc
OBJCOPY=x86_64-riria-objcopy

$CC "$SRC" -o "$BASENAME.elf"

$OBJCOPY -O binary "$BASENAME.elf" "$BASENAME.bin"

mkdir -p ../initramfs
mv "$BASENAME.bin" ../initramfs/
mv "$BASENAME.elf" ../initramfs/

