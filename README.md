# chess-boot-loader

A boot-themed two-player chess game. A BIOS-style splash screen plays on launch, then you play chess on a board where each piece is a programming language icon.

## Concept

| Piece  | Language   | Asset file         | Side indicator        |
|--------|------------|--------------------|-----------------------|
| King   | C          | `c.svg`            | Black or white circle |
| Queen  | Java       | `java.svg`         | Black or white circle |
| Bishop | Python     | `python.svg`       | Black or white circle |
| Knight | JavaScript | `javascript.svg`   | Black or white circle |
| Rook   | Rust       | `rust.svg`         | Black or white circle |
| Pawn   | Go         | `go.svg`             | Black or white circle |

Same icon on both sides — the circle background color tells them apart.

## Stack

- **Language:** C++17
- **Rendering:** SDL2, SDL2_image, SDL2_ttf
- **Build:** CMake + pkg-config
- **Primary target:** Linux

---

## Progress (Parts 1–10)

### Part 1 — Project skeleton ✅
- CMake project with C++17
- Folder layout: `src/`, `include/`, `assets/`, `assets/pieces/`
- Minimal `main.cpp` entry point

### Part 2 — SDL2 integration ✅
- SDL2 window (800×800, resizable)
- Game loop with event polling and clean shutdown
- Linked SDL2_image and SDL2_ttf

### Part 3 — Boot splash screen ✅
- `boot_splash.cpp` — BIOS-style green typewriter text
- POST lines and language init messages
- Skip with key/mouse or auto-continue after 4 seconds

### Part 4 — Asset loading ✅
- `asset_loader.cpp` — loads board JPG and piece icons
- Clear error messages for missing files
- Textures freed on shutdown

### Part 5 — Board rendering ✅
- `board_renderer.cpp` — 8×8 grid mapped to screen coordinates
- Board image scaled with aspect ratio preserved
- `--debug` flag shows square borders and a–h / 1–8 labels

### Part 6 — Piece icon setup ✅
- `chess_types.h` — `PieceType`, `Color`, `Piece`
- `piece_renderer.cpp` — circle background + language icon
- SVG icons with text fallback if assets are missing

### Part 7 — Board state & starting position ✅
- `chess_board.cpp` — 8×8 board array
- `reset_to_starting_position()` with standard chess setup
- Renders all 32 pieces from board state

### Part 8 — Square selection input ✅
- Click a square to highlight it
- HiDPI mouse coordinate scaling
- Clicks outside the board are ignored

### Part 9 — Move selection flow ✅
- `game_controller.cpp` — two-click move flow
- First click selects your piece (current player only)
- Second click moves to destination (or re-selects a friendly piece)
- Click same square again to deselect
- Turns alternate White → Black
- Yellow highlight on selected piece, green on destination squares

### Part 10 — Basic move validation ✅
- `move_validator.cpp` — per-piece movement rules
- Pawn: single/double push from start, diagonal capture only
- Knight: L-shaped jumps
- Bishop/Rook/Queen: sliding with path blocking
- King: one square any direction
- Blocks moves onto friendly pieces; captures remove enemy pieces
- Illegal moves rejected with no board change
- Green highlights show only legal destination squares

### Alignment fix ✅
- Board grid insets measured from `chess-board-banner-vector.jpg` (1920×1920)
- Playable area: origin 48px, size 1824px on each side (`board_layout.h`)

---

## Build & run

Install dependencies on Linux:

```bash
sudo apt install build-essential cmake pkg-config \
  libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev librsvg2-dev
```

Build and run from the project root:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/chess-boot-loader
```

Debug mode (square grid overlay):

```bash
./build/chess-boot-loader --debug
```

---

## Project structure

```
chess-boot-loader/
├── CMakeLists.txt
├── README.md
├── assets/
│   ├── chess-board-banner-vector.jpg
│   └── pieces/
│       ├── c.svg
│       ├── java.svg
│       ├── python.svg
│       ├── javascript.svg
│       ├── rust.svg
│       └── go.svg
├── include/
│   ├── asset_loader.h
│   ├── board_layout.h
│   ├── board_renderer.h
│   ├── boot_splash.h
│   ├── chess_board.h
│   ├── chess_types.h
│   ├── game_controller.h
│   ├── move_validator.h
│   └── piece_renderer.h
└── src/
    ├── main.cpp
    ├── asset_loader.cpp
    ├── board_renderer.cpp
    ├── boot_splash.cpp
    ├── chess_board.cpp
    ├── chess_types.cpp
    ├── game_controller.cpp
    ├── move_validator.cpp
    └── piece_renderer.cpp
```

---

## Remaining parts (11–15)

| Part | Description                                      | Status  |
|------|--------------------------------------------------|---------|
| 11   | Advanced rules (check, checkmate, promotion)     | Pending |
| 12   | Game UI (turn indicator, checkmate screen)       | Pending |
| 13   | Boot-to-game transition polish                   | Pending |
| 14   | Linux build verification                         | Pending |
| 15   | Final polish & release                           | Pending |

---

## How to play (current build)

1. Boot splash plays (click or wait to skip).
2. White moves first.
3. Click one of your pieces to select it.
4. Click a destination square to move.
5. Click the same piece again to deselect.
6. Click another friendly piece while selected to switch selection.
7. Turns alternate after each move.
8. Green squares show legal moves for the selected piece.

Basic chess rules apply (piece movement, captures, blocked paths). Check/checkmate validation comes in Part 11.

---

## Future ideas

- AI opponent (minimax)
- Move history (algebraic notation)
- Sound effects
- Bare-metal bootloader version (separate project)
