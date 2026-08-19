#!/usr/bin/env bash
# Install build tools for bare-metal chess-boot-loader (macOS / Homebrew)

set -euo pipefail

command -v brew >/dev/null || {
    echo "Install Homebrew first: https://brew.sh"
    exit 1
}

brew install nasm qemu xorriso i686-elf-grub i686-elf-gcc librsvg

echo
echo "Build and run:"
echo "  cd bare-metal"
echo "  make iso"
echo "  ./scripts/run-qemu.sh"
