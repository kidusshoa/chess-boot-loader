#include "square_selection.h"

#include "board_renderer.h"

SquareSelection::SquareSelection() : has_selection_(false), file_(0), rank_(0) {}

bool SquareSelection::has_selection() const {
    return has_selection_;
}

int SquareSelection::selected_file() const {
    return file_;
}

int SquareSelection::selected_rank() const {
    return rank_;
}

void SquareSelection::clear() {
    has_selection_ = false;
}

bool SquareSelection::handle_click(const BoardRenderer& board, int x, int y) {
    int file = 0;
    int rank = 0;
    if (!board.pixel_to_square(x, y, file, rank)) {
        return false;
    }

    has_selection_ = true;
    file_ = file;
    rank_ = rank;
    return true;
}

void SquareSelection::draw_highlight(SDL_Renderer* renderer, const BoardRenderer& board) const {
    if (!has_selection_) {
        return;
    }

    const SDL_Rect square = board.square_bounds(file_, rank_);
    if (square.w <= 0 || square.h <= 0) {
        return;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 96);
    SDL_RenderFillRect(renderer, &square);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}
