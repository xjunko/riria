#!/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC_DIR="$SCRIPT_DIR/initramfs"

OUT_FILE="$(realpath "$SCRIPT_DIR/../base/boot/initramfs.tar")"

if [ ! -d "$SRC_DIR" ]; then
    echo "Error: Source directory '$SRC_DIR' does not exist."
    exit 1
fi

OUT_DIR="$(dirname "$OUT_FILE")"
mkdir -p "$OUT_DIR"

echo "generating initramfs..."
tar -C "$SRC_DIR" --blocking-factor 1 -cf "$OUT_FILE" .
echo "initramfs synced!"
