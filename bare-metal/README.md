# Bare-metal chess-boot-loader

Real x86 kernel that boots through **GRUB Multiboot2** in **QEMU** — not a mock SDL splash.

The desktop chess game in the repo root (`src/`, SDL2) and this kernel are **two separate programs**:

| | Desktop app (`/`) | Bare-metal kernel (`bare-metal/`) |
|---|-------------------|-----------------------------------|
| Runs in | Linux (SDL window) | QEMU / real hardware (no OS) |
| Boot | Fake BIOS text (graphics) | GRUB → your kernel |
| Chess | Full game | Boot log only (chess port next) |

---

## What this boots

1. **GRUB** loads `kernel.bin` (Multiboot2)
2. **`boot.asm`** sets up stack, requests 1024×768 framebuffer
3. **`boot_log.c`** prints POST-style text on VGA
4. **`framebuffer.c` + `gfx.c`** draw the chess board in graphics mode
5. **`ui.c` + `chess.c`** run a playable two-player game with keyboard input

Piece icons come from **`assets/pieces/*.svg`**, converted at build time to embedded RGBA bitmaps (`generated/assets_gen.c`). If `rsvg-convert` is missing, placeholder sprites are used instead.

---

## Controls (QEMU window must have focus)

| Key | Action |
|-----|--------|
| Arrow keys | Move cursor |
| Space / Enter | Select piece or move |
| R | Restart game |

---

## Requirements (Linux)

```bash
./scripts/install-deps-linux.sh
```

## Requirements (macOS)

```bash
./scripts/install-deps-macos.sh
```

Or manually:

```bash
brew install nasm qemu xorriso i686-elf-grub i686-elf-gcc librsvg
```

`librsvg` provides `rsvg-convert` for embedding real piece icons at build time (optional).

---

Or manually:

```bash
sudo apt install build-essential nasm grub-pc-bin xorriso \
  qemu-system-x86 gcc-multilib
```

---

## Build & run in QEMU

```bash
cd bare-metal
make iso
./scripts/run-qemu.sh
```

Or:

```bash
make qemu
```

You should see green POST text, then a **graphical chess board** with language-themed piece icons.

---

## Cross compiler (optional)

If `gcc -m32` fails on your machine:

```bash
sudo apt install gcc-i686-linux-gnu binutils-i686-linux-gnu
make TARGET=i686-elf iso
```

Install an `i686-elf` toolchain from OSDev if you prefer a fully freestanding compiler.

---

## Project layout

```
bare-metal/
├── linker.ld
├── grub.cfg
├── Makefile
├── generated/          # assets_gen.c (build output)
├── include/
│   ├── kernel.h
│   ├── multiboot2.h
│   ├── framebuffer.h
│   ├── gfx.h
│   ├── font.h
│   ├── bitmap.h
│   ├── assets.h
│   ├── chess.h
│   ├── input.h
│   ├── ui.h
│   ├── vga.h
│   └── boot_log.h
├── src/
│   ├── boot.asm
│   ├── kernel.c
│   ├── boot_log.c
│   ├── framebuffer.c
│   ├── gfx.c
│   ├── font.c
│   ├── chess.c
│   ├── input.c
│   ├── ui.c
│   └── vga.c
└── scripts/
    ├── generate_assets.py
    ├── run-qemu.sh
    ├── install-deps-linux.sh
    └── install-deps-macos.sh
```

---

## Roadmap

1. **Done** — Multiboot2 kernel + VGA boot log + GUI chess in QEMU
2. **Next** — PS/2 mouse click input
3. **Next** — Board background JPG as embedded bitmap
4. **Next** — Castling / en passant

The existing SDL chess code cannot be linked into this kernel. Chess **rules** can be reimplemented; **rendering** must use direct hardware/framebuffer.

---

## Troubleshooting

**`grub-mkrescue: not found`**  
Install `grub-pc-bin` and `xorriso`.

**`cannot find -lgcc` with `-m32`**  
Install `gcc-multilib` or use `TARGET=i686-elf`.

**Black screen in QEMU**  
Rebuild: `make clean && make iso`. Ensure `kernel.bin` exists in the ISO.

**Want serial debug**  
QEMU is already started with `-serial stdio` in `run-qemu.sh`.
