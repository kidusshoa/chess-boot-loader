#pragma once

#include "chess_types.h"

class BoardRenderer;
class ChessBoard;

#include <SDL2/SDL.h>

class GameController {
public:
    GameController();

    Color current_player() const;
    bool has_piece_selected() const;
    int selected_file() const;
    int selected_rank() const;

    bool handle_click(ChessBoard& board, const BoardRenderer& board_renderer, int x, int y);
    void draw_highlights(SDL_Renderer* renderer, const BoardRenderer& board_renderer) const;

private:
    Color current_player_;
    bool has_selection_;
    int from_file_;
    int from_rank_;

    void clear_selection();
    bool select_piece(const ChessBoard& board, int file, int rank);
    bool attempt_move(ChessBoard& board, int to_file, int to_rank);
};
