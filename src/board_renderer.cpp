#include "board_renderer.h"

#include "board_layout.h"

#include <iostream>
using namespace std;

#include <SDL2/SDL_ttf.h>

#include <algorithm>

namespace {

TTF_Font* load_debug_font() {
    const char* font_paths[] = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
        "/System/Library/Fonts/Menlo.ttc",
        "/Library/Fonts/Menlo.ttc",
    };

    for (const char* path : font_paths) {
        TTF_Font* font = TTF_OpenFont(path, 14);
        if (font) {
            return font;
        }
    }

    return nullptr;
}

void render_label(SDL_Renderer* renderer, TTF_Font* font, const char* text, int x, int y) {
    SDL_Color color = {255, 64, 64, 255};
    SDL_Surface* surface = TTF_RenderText_Blended(font, text, color);
    if (!surface) {
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) {
        return;
    }

    SDL_Rect dest = {x, y, 0, 0};
    SDL_QueryTexture(texture, nullptr, nullptr, &dest.w, &dest.h);
    SDL_RenderCopy(renderer, texture, nullptr, &dest);
    SDL_DestroyTexture(texture);
}

}  // namespace

BoardRenderer::BoardRenderer(bool debug_overlay)
    : debug_overlay_(debug_overlay),
      image_dest_{0, 0, 0, 0},
      board_area_{0, 0, 0, 0},
      squares_{} {}

void BoardRenderer::update_layout(int window_width, int window_height, int texture_width, int texture_height) {
    if (window_width <= 0 || window_height <= 0 || texture_width <= 0 || texture_height <= 0) {
        return;
    }

    const float scale = min(
        window_width / static_cast<float>(texture_width),
        window_height / static_cast<float>(texture_height)
    );

    const int dest_w = static_cast<int>(texture_width * scale);
    const int dest_h = static_cast<int>(texture_height * scale);

    image_dest_.x = (window_width - dest_w) / 2;
    image_dest_.y = (window_height - dest_h) / 2;
    image_dest_.w = dest_w;
    image_dest_.h = dest_h;

    board_area_.x = image_dest_.x + static_cast<int>(image_dest_.w * BOARD_GRID_ORIGIN_NORM);
    board_area_.y = image_dest_.y + static_cast<int>(image_dest_.h * BOARD_GRID_ORIGIN_NORM);
    board_area_.w = static_cast<int>(image_dest_.w * BOARD_GRID_SIZE_NORM);
    board_area_.h = static_cast<int>(image_dest_.h * BOARD_GRID_SIZE_NORM);

    compute_squares();
}

void BoardRenderer::compute_squares() {
    const int square_w = board_area_.w / 8;
    const int square_h = board_area_.h / 8;

    for (int rank = 0; rank < 8; ++rank) {
        for (int file = 0; file < 8; ++file) {
            squares_[rank][file].x = board_area_.x + file * square_w;
            squares_[rank][file].y = board_area_.y + (7 - rank) * square_h;
            squares_[rank][file].w = square_w;
            squares_[rank][file].h = square_h;
        }
    }
}

void BoardRenderer::draw(SDL_Renderer* renderer, SDL_Texture* board_texture) const {
    if (!board_texture || image_dest_.w <= 0 || image_dest_.h <= 0) {
        return;
    }

    SDL_RenderCopy(renderer, board_texture, nullptr, &image_dest_);
}

void BoardRenderer::draw_debug_overlay(SDL_Renderer* renderer) const {
    if (!debug_overlay_) {
        return;
    }

    SDL_SetRenderDrawColor(renderer, 255, 64, 64, 255);
    for (int rank = 0; rank < 8; ++rank) {
        for (int file = 0; file < 8; ++file) {
            SDL_RenderDrawRect(renderer, &squares_[rank][file]);
        }
    }

    static bool ttf_initialized = false;
    static TTF_Font* font = nullptr;

    if (!ttf_initialized) {
        if (TTF_Init() == 0) {
            font = load_debug_font();
        }
        ttf_initialized = true;
    }

    if (!font) {
        return;
    }

    for (int file = 0; file < 8; ++file) {
        const char label = static_cast<char>('a' + file);
        char text[2] = {label, '\0'};
        const SDL_Rect& bottom = squares_[0][file];
        render_label(renderer, font, text, bottom.x + 4, bottom.y + bottom.h - 18);
    }

    for (int rank = 0; rank < 8; ++rank) {
        const char text[2] = {static_cast<char>('1' + rank), '\0'};
        const SDL_Rect& left = squares_[rank][0];
        render_label(renderer, font, text, left.x + 4, left.y + 4);
    }
}

SDL_Rect BoardRenderer::square_bounds(int file, int rank) const {
    if (file < 0 || file > 7 || rank < 0 || rank > 7) {
        return {0, 0, 0, 0};
    }
    return squares_[rank][file];
}

bool BoardRenderer::pixel_to_square(int x, int y, int& file, int& rank) const {
    for (int r = 0; r < 8; ++r) {
        for (int f = 0; f < 8; ++f) {
            const SDL_Rect& square = squares_[r][f];
            if (x >= square.x && x < square.x + square.w && y >= square.y && y < square.y + square.h) {
                file = f;
                rank = r;
                return true;
            }
        }
    }
    return false;
}

bool BoardRenderer::debug_overlay() const {
    return debug_overlay_;
}
