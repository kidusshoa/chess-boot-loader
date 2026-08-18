#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${ROOT_DIR}"

echo "==> Building bare-metal ISO..."
make iso

echo "==> Launching QEMU (i386)..."
echo "    Close the QEMU window or press Ctrl+C here to stop."
qemu-system-i386 \
    -cdrom build/chess-boot-loader.iso \
    -m 128M \
    -serial stdio \
    -no-reboot \
    -no-shutdown
