#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${ROOT_DIR}"

echo "==> Building bare-metal ISO..."
make iso

SERIAL_LOG="${ROOT_DIR}/build/serial.log"
rm -f "${SERIAL_LOG}"

DISPLAY_ARG="-display cocoa"
if [[ "${CHESS_QEMU_DISPLAY:-}" == "sdl" ]]; then
    DISPLAY_ARG="-display sdl"
fi

echo "==> Launching QEMU (i386)..."
echo "    Serial log: ${SERIAL_LOG}"
echo "    Click the QEMU window so it has keyboard focus."
echo "    If keys still fail, try: CHESS_QEMU_DISPLAY=sdl ./scripts/run-qemu.sh"
echo "    Close the QEMU window or press Ctrl+C here to stop."
qemu-system-i386 \
    -machine pc \
    -cdrom build/chess-boot-loader.iso \
    -m 128M \
    -serial "file:${SERIAL_LOG}" \
    -vga std \
    ${DISPLAY_ARG} \
    -k en-us \
    -no-reboot \
    -no-shutdown

echo "==> Serial output:"
if [[ -s "${SERIAL_LOG}" ]]; then
    cat "${SERIAL_LOG}"
else
    echo "(empty - kernel may not have reached serial_init)"
fi
