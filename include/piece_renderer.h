#pragma once

#include "asset_loader.h"
#include "board_renderer.h"
#include "chess_types.h"

#include <SDL2/SDL.h>

class PieceRenderer {
public:
    static void load_standard_pieces(AssetLoader& assets);
    static void draw(
        SDL_Renderer* renderer,
        const AssetLoader& assets,
        const BoardRenderer& board,
        const Piece& piece,
        int file,
        int rank
    );
    static void draw_starting_position(
        SDL_Renderer* renderer,
        const AssetLoader& assets,
        const BoardRenderer& board
    );
};
