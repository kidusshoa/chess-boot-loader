#pragma once

#include <SDL2/SDL.h>

class BoardRenderer {
public:
    explicit BoardRenderer(bool debug_overlay = false);

    void update_layout(int window_width, int window_height, int texture_width, int texture_height);
    void draw(SDL_Renderer* renderer, SDL_Texture* board_texture) const;
    void draw_debug_overlay(SDL_Renderer* renderer) const;

    // file: 0-7 (a-h), rank: 0-7 (1-8, rank 0 is white's back rank at the bottom)
    SDL_Rect square_bounds(int file, int rank) const;
    bool pixel_to_square(int x, int y, int& file, int& rank) const;

    bool debug_overlay() const;

private:
    bool debug_overlay_;
    SDL_Rect image_dest_;
    SDL_Rect board_area_;
    SDL_Rect squares_[8][8];

    void compute_squares();
};
