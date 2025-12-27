#!/bin/env bash
set -e

if [ $# -lt 1 ]; then
    echo "Usage: $0 <file>"
    exit 1
fi

SRC="$1"
BASENAME=$(basename "$SRC" .c)

export PATH="/home/lain/Projects/Tooling/Cross/x86_64/gcc-x86_64-elf_15.2.0/bin/:$PATH"

CC=x86_64-elf-gcc
LD=x86_64-elf-ld
OBJCOPY=x86_64-elf-objcopy

$CC \
    -O0 -ffreestanding -fno-pie -fno-pic \
    -nostdlib -fno-stack-protector\
    -I./libc/include \
    -c "$SRC" -o "$BASENAME.o"

$LD -nostdlib -static -T link.ld -o "$BASENAME.elf" "$BASENAME.o" libc/libriria.a

$OBJCOPY -O binary "$BASENAME.elf" "$BASENAME.bin"

mv "$BASENAME.bin" ../initramfs/
mv "$BASENAME.elf" ../initramfs/

rm "$BASENAME.o"
