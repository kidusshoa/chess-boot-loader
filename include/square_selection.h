#pragma once

class BoardRenderer;

#include <SDL2/SDL.h>

class SquareSelection {
public:
    SquareSelection();

    bool has_selection() const;
    int selected_file() const;
    int selected_rank() const;

    void clear();
    bool handle_click(const BoardRenderer& board, int x, int y);
    void draw_highlight(SDL_Renderer* renderer, const BoardRenderer& board) const;

private:
    bool has_selection_;
    int file_;
    int rank_;
};
