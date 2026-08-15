#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

VERSION="$(grep CHESS_BOOT_LOADER_VERSION include/version.h | sed 's/.*"\(.*\)".*/\1/')"
ARCH="$(uname -m)"
RELEASE_NAME="chess-boot-loader-${VERSION}-linux-${ARCH}"
STAGE_DIR="${ROOT_DIR}/dist/${RELEASE_NAME}"
ARCHIVE="${ROOT_DIR}/dist/${RELEASE_NAME}.tar.gz"

echo "==> Building release ${RELEASE_NAME}..."
./scripts/build-linux.sh

echo "==> Staging release files..."
rm -rf "${STAGE_DIR}"
mkdir -p "${STAGE_DIR}/assets/pieces"

cp build/chess-boot-loader "${STAGE_DIR}/"
cp assets/chess-board-banner-vector.jpg "${STAGE_DIR}/assets/"
cp assets/pieces/*.svg "${STAGE_DIR}/assets/pieces/" 2>/dev/null || true

if compgen -G "build/assets/pieces/*.png" > /dev/null; then
    cp build/assets/pieces/*.png "${STAGE_DIR}/assets/pieces/"
fi

cp README.md LICENSE "${STAGE_DIR}/"
cp scripts/run-chess-boot-loader.sh "${STAGE_DIR}/chess-boot-loader.sh"
chmod +x "${STAGE_DIR}/chess-boot-loader.sh"

cat > "${STAGE_DIR}/RUN.txt" <<EOF
chess-boot-loader ${VERSION}

Run:
  ./chess-boot-loader.sh

Or directly:
  ./chess-boot-loader

Requires on the target machine:
  libsdl2-2.0-0 libsdl2-image-2.0-0 libsdl2-ttf-2.0-0 librsvg2-2

Install runtime deps (Debian/Ubuntu):
  sudo apt install libsdl2-2.0-0 libsdl2-image-2.0-0 libsdl2-ttf-2.0-0 librsvg2-2
EOF

echo "==> Creating archive..."
mkdir -p "${ROOT_DIR}/dist"
tar -czf "${ARCHIVE}" -C "${ROOT_DIR}/dist" "${RELEASE_NAME}"

echo
echo "Release ready:"
echo "  Folder:  ${STAGE_DIR}"
echo "  Archive: ${ARCHIVE}"
echo
echo "Test the release:"
echo "  cd ${STAGE_DIR} && ./chess-boot-loader.sh"
