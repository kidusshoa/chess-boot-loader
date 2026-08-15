#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

echo "==> Checking Linux build dependencies..."
for pkg in sdl2 SDL2_image SDL2_ttf; do
    pkg-config --exists "$pkg"
done

if ! pkg-config --exists librsvg-2.0 2>/dev/null; then
    echo "Warning: librsvg-2.0 not found. SVG piece icons will not load."
    echo "Install with: sudo apt install librsvg2-dev libcairo2-dev"
fi

if ! command -v rsvg-convert >/dev/null 2>&1; then
    echo "Warning: rsvg-convert not found. Build will skip PNG pre-rasterization."
    echo "Install with: sudo apt install librsvg2-bin"
fi

echo "==> Configuring..."
cmake -B build -DCMAKE_BUILD_TYPE=Release

echo "==> Building..."
cmake --build build

echo "==> Done."
echo "Run from project root:"
echo "  ./build/chess-boot-loader"
echo "Or from build directory:"
echo "  cd build && ./chess-boot-loader"
