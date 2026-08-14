#pragma once

#include "chess_types.h"

class BoardRenderer;
class ChessBoard;

#include <SDL2/SDL.h>

enum class GameResult {
    InProgress,
    WhiteWins,
    BlackWins,
    Draw,
};

class GameController {
public:
    GameController();

    Color current_player() const;
    GameResult game_result() const;
    bool is_in_check() const;
    bool has_piece_selected() const;
    int selected_file() const;
    int selected_rank() const;
    bool has_last_move() const;
    int last_move_from_file() const;
    int last_move_from_rank() const;
    int last_move_to_file() const;
    int last_move_to_rank() const;

    void restart(ChessBoard& board);
    bool handle_click(ChessBoard& board, const BoardRenderer& board_renderer, int x, int y);
    void draw_highlights(
        SDL_Renderer* renderer,
        const ChessBoard& board,
        const BoardRenderer& board_renderer
    ) const;

private:
    Color current_player_;
    GameResult game_result_;
    bool in_check_;
    bool has_selection_;
    int from_file_;
    int from_rank_;
    bool has_last_move_;
    int last_from_file_;
    int last_from_rank_;
    int last_to_file_;
    int last_to_rank_;

    void clear_selection();
    void clear_last_move();
    void update_game_state(const ChessBoard& board);
    bool select_piece(const ChessBoard& board, int file, int rank);
    bool attempt_move(ChessBoard& board, int to_file, int to_rank);
    void draw_check_highlight(
        SDL_Renderer* renderer,
        const ChessBoard& board,
        const BoardRenderer& board_renderer
    ) const;
    void draw_last_move_highlight(SDL_Renderer* renderer, const BoardRenderer& board_renderer) const;
};
