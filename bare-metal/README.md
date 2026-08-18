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
2. **`boot.asm`** sets up stack, jumps to C
3. **`kernel.c`** checks Multiboot magic
4. **`boot_log.c`** prints a real POST-style sequence to **VGA text mode** (`0xB8000`)
5. CPU halts — chess rendering for bare metal comes in a later step

This is a **real** boot path: no Linux, no SDL, freestanding C only.

---

## Requirements (Linux)

```bash
./scripts/install-deps-linux.sh
```

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

You should see green POST text in the QEMU window, ending with `Boot complete. Chess runtime is next.`

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
├── linker.ld           # kernel linked at 1 MiB
├── grub.cfg            # GRUB menu → multiboot2
├── Makefile
├── include/
│   ├── kernel.h
│   ├── vga.h
│   └── boot_log.h
├── src/
│   ├── boot.asm        # Multiboot2 header + entry
│   ├── kernel.c
│   ├── vga.c           # VGA text driver
│   └── boot_log.c      # POST boot sequence
└── scripts/
    ├── run-qemu.sh
    └── install-deps-linux.sh
```

---

## Roadmap to chess on bare metal

1. **Done** — Multiboot2 kernel + VGA boot log in QEMU  
2. **Next** — PS/2 keyboard input  
3. **Next** — VGA mode 13h or linear framebuffer board  
4. **Next** — Port chess logic (reuse rules from `../src/move_validator.cpp` ideas, rewrite freestanding)  
5. **Next** — Blit language icons as raw bitmaps (no SVG/librsvg in kernel)

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
