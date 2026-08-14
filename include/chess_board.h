#pragma once

#include "chess_types.h"

#include <SDL2/SDL.h>

class AssetLoader;
class BoardRenderer;

class ChessBoard {
public:
    ChessBoard();

    void reset_to_starting_position();

    const Piece& at(int file, int rank) const;
    Piece& at(int file, int rank);

    void draw(SDL_Renderer* renderer, const AssetLoader& assets, const BoardRenderer& board_renderer) const;

private:
    Piece squares_[8][8];
};
