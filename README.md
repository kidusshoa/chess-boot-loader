# chess-boot-loader

A boot-themed two-player chess game. A BIOS-style splash screen plays on launch, then you play chess on a board where each piece is a programming language icon.

**Version:** 0.1.0 · **License:** GPL-3.0 (see [LICENSE](LICENSE))

## Concept

| Piece  | Language   | Asset file         | Side indicator        |
|--------|------------|--------------------|-----------------------|
| King   | C          | `c.svg`            | Black or white circle |
| Queen  | Java       | `java.svg`         | Black or white circle |
| Bishop | Python     | `python.svg`       | Black or white circle |
| Knight | JavaScript | `javascript.svg`   | Black or white circle |
| Rook   | Rust       | `rust.svg`         | Black or white circle |
| Pawn   | Go         | `go.svg`           | Black or white circle |

Same icon on both sides — the circle background color tells them apart.

## Stack

- **Language:** C++17
- **Rendering:** SDL2, SDL2_image, SDL2_ttf
- **Build:** CMake + pkg-config
- **Target:** Linux

---

## How it works

### Boot splash

On launch, a fullscreen black screen shows green BIOS-style text (`boot_splash.cpp`): POST checks, memory init, and loading messages for each language piece. Skip anytime with a key press or mouse click, or wait ~4 seconds.

The boot text fades out, then the chess board fades in. The boot sequence runs once per app launch — restarting a game with **R** does not replay it.

Skip the boot splash for faster iteration:

```bash
./build/chess-boot-loader --no-boot
# or
CHESS_BOOT_LOADER_NO_BOOT=1 ./build/chess-boot-loader
```

### Board & pieces

The board image is loaded from `assets/chess-board-banner-vector.jpg` and scaled to fit the window while keeping its aspect ratio. An 8×8 grid is mapped onto the image using measured pixel bounds (origin 48px, playable area 1824×1824 on the 1920×1920 source — see `board_layout.h`).

Each piece is drawn as a language SVG icon on a colored circle background (`piece_renderer.cpp`). White pieces use a light circle; black pieces use a dark circle. If an SVG fails to load, a text label fallback is shown instead.

### Game flow

`game_controller.cpp` handles input and turn logic:

1. White moves first.
2. First click selects one of your pieces (yellow highlight).
3. Green squares show legal destinations for that piece.
4. Second click moves to the destination, or click another friendly piece to switch selection.
5. Click the same piece again to deselect.
6. Turns alternate after each valid move.

Clicks outside the board are ignored. Mouse coordinates are scaled for resizable / HiDPI windows.

### Chess rules

`move_validator.cpp` enforces standard piece movement:

- **Pawn** — one or two squares forward from start, diagonal capture only
- **Knight** — L-shaped jumps
- **Bishop / Rook / Queen** — sliding, blocked by pieces in the path
- **King** — one square in any direction

Additional rules:

- Cannot move onto a friendly piece
- Captures remove the enemy piece
- Cannot move if it leaves your king in check
- **Checkmate** ends the game (winner stored in `GameController`)
- **Stalemate** ends the game as a draw
- **Pawn promotion** — auto-promotes to Queen on the back rank
- Pieces with no legal moves cannot be selected

### Game UI

`game_ui.cpp` draws a status bar at the top of the window:

- **Turn indicator** — `White to move` / `Black to move`
- **Check** — appends `Check!` when the current player's king is threatened
- **Game over** — full-screen overlay for checkmate or stalemate, with `Press R to restart`
- **King in check** — red highlight on the king's square
- **Last move** — blue tint on the from/to squares of the previous move

Press **R** anytime after game over to reset the board and play again.

### Debug mode

Pass `--debug` to draw square borders and a–h / 1–8 labels over the board, useful for verifying grid alignment:

```bash
./build/chess-boot-loader --debug
```

---

## Build & run

### Linux

Install dependencies:

```bash
sudo apt install build-essential cmake pkg-config \
  libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev librsvg2-dev
```

Clone, build, and run:

```bash
git clone <your-repo-url> chess-boot-loader
cd chess-boot-loader
./scripts/build-linux.sh
./build/chess-boot-loader
```

Or manually:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/chess-boot-loader
```

The build copies `assets/` next to the binary automatically, so these all work:

```bash
./build/chess-boot-loader              # from project root
cd build && ./chess-boot-loader        # from build directory
```

### Asset path

Assets are searched in this order:

1. `$CHESS_BOOT_LOADER_ASSETS` (if set)
2. `assets/` (project root)
3. `../assets/` (when running from `build/`)
4. `./assets/` (next to the binary — populated by CMake)

On startup the game prints which asset directory it found, e.g. `Using assets from: assets`.

### Flags

| Flag           | Description                          |
|----------------|--------------------------------------|
| `--debug`      | Show square grid overlay             |
| `--no-boot`    | Skip the BIOS boot splash            |
| `--help`, `-h` | Show usage and exit                  |
| `--version`, `-v` | Show version and exit             |

Environment variables:

| Variable                   | Description                    |
|----------------------------|--------------------------------|
| `CHESS_BOOT_LOADER_NO_BOOT`| Skip boot splash (`1` to skip) |
| `CHESS_BOOT_LOADER_ASSETS` | Custom path to assets folder   |

### Troubleshooting on Linux

**Board or pieces not loading**
- Run from project root or `build/` as shown above
- Confirm assets exist: `ls assets/pieces/*.svg`
- Check startup output for `Using assets from: ...`

**SVG icons show text fallback instead**
- Install SVG support: `sudo apt install librsvg2-dev`
- Reinstall SDL2_image: `sudo apt install --reinstall libsdl2-image-dev`

**No text in UI or boot splash**
- Install fonts: `sudo apt install fonts-dejavu fonts-liberation`

**Square grid misaligned with board**
- Run with `--debug` and verify red grid lines match the board squares
- Grid bounds are defined in `include/board_layout.h`

### Pull and test on another machine

```bash
git pull
./scripts/build-linux.sh
./build/chess-boot-loader --no-boot
```

Play a short game, then press **R** to verify restart works.

---

## Known limitations (v0.1.0)

- Two-player local only — no AI or network play
- Pawn promotion always becomes Queen (no underpromotion)
- No castling or en passant
- No move history or undo
- Board grid alignment is tuned for the bundled board JPG — other board images may need adjusted bounds in `board_layout.h`
- SVG piece icons require SDL2_image built with librsvg support

---

## Release

This is **v0.1.0** — a playable local two-player chess game with boot splash, language-themed pieces, and standard rules (minus castling/en passant).

To tag a release after verifying on your Linux machine:

```bash
git tag -a v0.1.0 -m "chess-boot-loader v0.1.0"
git push origin v0.1.0
```

---

## Project structure

```
chess-boot-loader/
├── CMakeLists.txt
├── README.md
├── scripts/
│   └── build-linux.sh      # configure + build helper for Linux
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
│   ├── asset_loader.h      # texture loading (board + SVG pieces)
│   ├── board_layout.h      # measured grid bounds for the board image
│   ├── board_renderer.h    # board drawing and square mapping
│   ├── boot_splash.h       # BIOS-style startup screen
│   ├── chess_board.h       # 8×8 board state
│   ├── chess_types.h       # PieceType, Color, Piece
│   ├── game_controller.h   # input, turns, game result
│   ├── game_ui.h           # status bar and game-over overlay
│   ├── move_validator.h    # move legality and check detection
│   ├── piece_renderer.h    # language icon rendering
│   └── version.h           # release version string
└── src/
    ├── main.cpp
    ├── asset_loader.cpp
    ├── board_renderer.cpp
    ├── boot_splash.cpp
    ├── chess_board.cpp
    ├── chess_types.cpp
    ├── game_controller.cpp
    ├── game_ui.cpp
    ├── move_validator.cpp
    └── piece_renderer.cpp
```

---

## How to play

1. Boot splash plays — click or wait to skip.
2. White moves first.
3. Click one of your pieces to select it.
4. Click a green highlighted square to move.
5. Click the same piece again to deselect, or click another friendly piece to switch selection.
6. Turns alternate after each move.
7. You cannot move into check. Checkmate and stalemate end the game.
8. Pawns auto-promote to Queen on the back rank.
9. Press **R** to restart after checkmate or stalemate.

---

## Future ideas

- AI opponent (minimax)
- Move history (algebraic notation)
- Sound effects
- Castling and en passant
- Bare-metal bootloader version (separate project)
