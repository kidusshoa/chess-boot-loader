#!/usr/bin/env python3
"""Generate embedded RGBA bitmaps for bare-metal chess piece icons."""

from __future__ import annotations

import struct
import subprocess
import sys
import zlib
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SVG_DIR = ROOT / "assets" / "pieces"
OUT_DIR = Path(__file__).resolve().parents[1] / "generated"
BUILD_ASSETS = Path(__file__).resolve().parents[1] / "build" / "assets"

PIECES = [
    ("c", "PIECE_KING"),
    ("java", "PIECE_QUEEN"),
    ("python", "PIECE_BISHOP"),
    ("javascript", "PIECE_KNIGHT"),
    ("rust", "PIECE_ROOK"),
    ("go", "PIECE_PAWN"),
]

SIZE = 56


def png_to_rgba(path: Path) -> tuple[int, int, list[int]]:
    data = path.read_bytes()
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        raise ValueError(f"Not a PNG: {path}")

    width = height = 0
    bit_depth = color_type = 0
    idat = bytearray()
    offset = 8

    while offset + 8 <= len(data):
        length = struct.unpack(">I", data[offset : offset + 4])[0]
        chunk_type = data[offset + 4 : offset + 8]
        chunk_data = data[offset + 8 : offset + 8 + length]
        offset += 12 + length

        if chunk_type == b"IHDR":
            width, height, bit_depth, color_type = struct.unpack(">IIBB", chunk_data[:10])
        elif chunk_type == b"IDAT":
            idat.extend(chunk_data)

    if not idat:
        raise ValueError(f"PNG missing IDAT: {path}")

    raw = zlib.decompress(bytes(idat))
    stride = width * 4 + 1
    pixels: list[int] = []

    previous = bytearray(width * 4)
    index = 0

    for _y in range(height):
        filter_type = raw[index]
        index += 1
        row = bytearray(raw[index : index + width * 4])
        index += width * 4

        if filter_type == 1:
            for i in range(4, len(row)):
                row[i] = (row[i] + row[i - 4]) & 0xFF
        elif filter_type == 2:
            for i in range(len(row)):
                row[i] = (row[i] + previous[i]) & 0xFF
        elif filter_type == 3:
            for i in range(len(row)):
                left = row[i - 4] if i >= 4 else 0
                up = previous[i]
                row[i] = (row[i] + (left + up) // 2) & 0xFF
        elif filter_type == 4:
            for i in range(len(row)):
                a = row[i - 4] if i >= 4 else 0
                b = previous[i]
                c = previous[i - 4] if i >= 4 else 0
                p = a + b - c
                pa = abs(p - a)
                pb = abs(p - b)
                pc = abs(p - c)
                pr = a if pa <= pb and pa <= pc else (b if pb <= pc else c)
                row[i] = (row[i] + pr) & 0xFF

        if color_type == 6 and bit_depth == 8:
            for i in range(0, len(row), 4):
                r, g, b, a = row[i : i + 4]
                pixels.append((a << 24) | (r << 16) | (g << 8) | b)
        elif color_type == 2 and bit_depth == 8:
            for i in range(0, len(row), 3):
                r, g, b = row[i : i + 3]
                pixels.append(0xFF000000 | (r << 16) | (g << 8) | b)
        elif color_type == 0 and bit_depth == 8:
            for value in row:
                pixels.append(0xFF000000 | (value << 16) | (value << 8) | value)
        else:
            raise ValueError(f"Unsupported PNG format in {path}")

        previous = row

    return width, height, pixels


def make_placeholder(name: str) -> tuple[int, int, list[int]]:
    pixels: list[int] = []
    cx = cy = SIZE // 2
    radius = SIZE // 2 - 2

    for y in range(SIZE):
        for x in range(SIZE):
            dx = x - cx
            dy = y - cy
            inside = dx * dx + dy * dy <= radius * radius
            if inside:
                pixels.append(0xFF303040)
            else:
                pixels.append(0x00000000)

    return SIZE, SIZE, pixels


def svg_to_png(svg_path: Path, png_path: Path) -> bool:
    if not shutil_which("rsvg-convert"):
        return False

    png_path.parent.mkdir(parents=True, exist_ok=True)
    subprocess.run(
        [
            "rsvg-convert",
            "-w",
            str(SIZE),
            "-h",
            str(SIZE),
            str(svg_path),
            "-o",
            str(png_path),
        ],
        check=True,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )
    return png_path.is_file()


def shutil_which(command: str) -> str | None:
    from shutil import which

    return which(command)


def emit_c(name: str, enum_name: str, width: int, height: int, pixels: list[int]) -> str:
    lines = [
        f"static const uint32_t asset_{name}_pixels[] = {{",
    ]

    for index, pixel in enumerate(pixels):
        if index % 8 == 0:
            lines.append("    ")
        lines[-1] += f"0x{pixel:08X}, "
        if index % 8 == 7:
            lines[-1] = lines[-1].rstrip()

    if lines[-1].endswith(", "):
        lines[-1] = lines[-1].rstrip()

    lines.extend(
        [
            "",
            "};",
            "",
            f"static const bitmap_t asset_{name} = {{",
            f"    {width},",
            f"    {height},",
            f"    asset_{name}_pixels,",
            "};",
            "",
        ]
    )

    return "\n".join(lines)


def main() -> int:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    BUILD_ASSETS.mkdir(parents=True, exist_ok=True)

    body: list[str] = [
        '#include "assets.h"',
        "",
        "#include <stdint.h>",
        "",
        "#include \"bitmap.h\"",
        "",
    ]

    switch_cases: list[str] = []

    for slug, enum_name in PIECES:
        svg_path = SVG_DIR / f"{slug}.svg"
        png_path = BUILD_ASSETS / f"{slug}.png"

        if svg_path.is_file() and svg_to_png(svg_path, png_path):
            width, height, pixels = png_to_rgba(png_path)
        else:
            width, height, pixels = make_placeholder(slug)

        body.append(emit_c(slug, enum_name, width, height, pixels))
        switch_cases.append(f"        case {enum_name}:\n            return &asset_{slug};")

    body.extend(
        [
            "const bitmap_t* assets_piece_bitmap(piece_type_t type) {",
            "    switch (type) {",
            *switch_cases,
            "        case PIECE_NONE:",
            "        default:",
            "            return 0;",
            "    }",
            "}",
            "",
        ]
    )

    output = OUT_DIR / "assets_gen.c"
    output.write_text("\n".join(body), encoding="utf-8")
    print(f"Wrote {output}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
