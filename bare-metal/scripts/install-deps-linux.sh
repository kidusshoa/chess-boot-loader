#!/usr/bin/env bash
# Install build tools for bare-metal chess-boot-loader (Debian/Ubuntu)

set -euo pipefail

sudo apt update
sudo apt install -y \
    build-essential \
    nasm \
    grub-pc-bin \
    grub-common \
    xorriso \
    qemu-system-x86 \
    gcc-multilib \
    g++-multilib

echo
echo "Optional cross toolchain (cleaner than gcc-multilib):"
echo "  sudo apt install gcc-i686-linux-gnu binutils-i686-linux-gnu"
echo
echo "Then build with:"
echo "  cd bare-metal && make iso && make qemu"
