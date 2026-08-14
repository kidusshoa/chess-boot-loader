#include "game_controller.h"

#include "board_renderer.h"
#include "chess_board.h"

GameController::GameController()
    : current_player_(Color::White),
      has_selection_(false),
      from_file_(0),
      from_rank_(0) {}

Color GameController::current_player() const {
    return current_player_;
}

bool GameController::has_piece_selected() const {
    return has_selection_;
}

int GameController::selected_file() const {
    return from_file_;
}

int GameController::selected_rank() const {
    return from_rank_;
}

void GameController::clear_selection() {
    has_selection_ = false;
}

bool GameController::select_piece(const ChessBoard& board, int file, int rank) {
    const Piece& piece = board.at(file, rank);
    if (piece.is_empty() || piece.color != current_player_) {
        return false;
    }

    has_selection_ = true;
    from_file_ = file;
    from_rank_ = rank;
    return true;
}

bool GameController::attempt_move(ChessBoard& board, int to_file, int to_rank) {
    if (!has_selection_) {
        return false;
    }

    if (to_file == from_file_ && to_rank == from_rank_) {
        clear_selection();
        return true;
    }

    const Piece& destination = board.at(to_file, to_rank);
    if (!destination.is_empty() && destination.color == current_player_) {
        return select_piece(board, to_file, to_rank);
    }

    board.move_piece(from_file_, from_rank_, to_file, to_rank);
    clear_selection();
    current_player_ = current_player_ == Color::White ? Color::Black : Color::White;
    return true;
}

bool GameController::handle_click(ChessBoard& board, const BoardRenderer& board_renderer, int x, int y) {
    int file = 0;
    int rank = 0;
    if (!board_renderer.pixel_to_square(x, y, file, rank)) {
        return false;
    }

    if (!has_selection_) {
        return select_piece(board, file, rank);
    }

    return attempt_move(board, file, rank);
}

void GameController::draw_highlights(SDL_Renderer* renderer, const BoardRenderer& board_renderer) const {
    if (!has_selection_) {
        return;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    for (int rank = 0; rank < 8; ++rank) {
        for (int file = 0; file < 8; ++file) {
            const SDL_Rect square = board_renderer.square_bounds(file, rank);
            if (square.w <= 0 || square.h <= 0) {
                continue;
            }

            if (file == from_file_ && rank == from_rank_) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 0, 110);
            } else {
                SDL_SetRenderDrawColor(renderer, 80, 200, 120, 70);
            }

            SDL_RenderFillRect(renderer, &square);
        }
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}
