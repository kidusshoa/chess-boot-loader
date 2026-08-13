#include "piece_renderer.h"

#include <iostream>
using namespace std;

#include <SDL2/SDL_ttf.h>

#include <algorithm>
#include <cmath>

namespace {

constexpr float ICON_SCALE = 0.80f;

TTF_Font* fallback_font() {
    static bool attempted = false;
    static TTF_Font* font = nullptr;

    if (attempted) {
        return font;
    }

    attempted = true;
    if (TTF_Init() != 0) {
        return nullptr;
    }

    const char* font_paths[] = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf",
        "/System/Library/Fonts/Supplemental/Arial Bold.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
    };

    for (const char* path : font_paths) {
        font = TTF_OpenFont(path, 16);
        if (font) {
            break;
        }
    }

    return font;
}

void fill_circle(SDL_Renderer* renderer, int center_x, int center_y, int radius) {
    for (int y = -radius; y <= radius; ++y) {
        for (int x = -radius; x <= radius; ++x) {
            if (x * x + y * y <= radius * radius) {
                SDL_RenderDrawPoint(renderer, center_x + x, center_y + y);
            }
        }
    }
}

void stroke_circle(SDL_Renderer* renderer, int center_x, int center_y, int radius) {
    for (int angle = 0; angle < 360; ++angle) {
        const double radians = angle * M_PI / 180.0;
        const int x = center_x + static_cast<int>(std::cos(radians) * radius);
        const int y = center_y + static_cast<int>(std::sin(radians) * radius);
        SDL_RenderDrawPoint(renderer, x, y);
    }
}

void draw_centered_texture(SDL_Renderer* renderer, SDL_Texture* texture, const SDL_Rect& square, int size) {
    int texture_width = 0;
    int texture_height = 0;
    SDL_QueryTexture(texture, nullptr, nullptr, &texture_width, &texture_height);

    const SDL_Rect dest = {
        square.x + (square.w - size) / 2,
        square.y + (square.h - size) / 2,
        size,
        size,
    };

    SDL_RenderCopy(renderer, texture, nullptr, &dest);
}

void draw_fallback_label(SDL_Renderer* renderer, const SDL_Rect& square, Color color, PieceType type) {
    TTF_Font* font = fallback_font();
    if (!font) {
        return;
    }

    SDL_Color text_color = color == Color::White ? SDL_Color{20, 20, 20, 255} : SDL_Color{240, 240, 240, 255};
    SDL_Surface* surface = TTF_RenderText_Blended(font, piece_fallback_label(type), text_color);
    if (!surface) {
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) {
        return;
    }

    int texture_width = 0;
    int texture_height = 0;
    SDL_QueryTexture(texture, nullptr, nullptr, &texture_width, &texture_height);

    const SDL_Rect dest = {
        square.x + (square.w - texture_width) / 2,
        square.y + (square.h - texture_height) / 2,
        texture_width,
        texture_height,
    };

    SDL_RenderCopy(renderer, texture, nullptr, &dest);
    SDL_DestroyTexture(texture);
}

}  // namespace

void PieceRenderer::load_standard_pieces(AssetLoader& assets) {
    const PieceType types[] = {
        PieceType::King,
        PieceType::Queen,
        PieceType::Bishop,
        PieceType::Knight,
        PieceType::Rook,
        PieceType::Pawn,
    };

    for (PieceType type : types) {
        const char* filename = piece_asset_file(type);
        if (filename) {
            assets.load_piece(filename);
        }
    }
}

void PieceRenderer::draw(
    SDL_Renderer* renderer,
    const AssetLoader& assets,
    const BoardRenderer& board,
    const Piece& piece,
    int file,
    int rank
) {
    if (piece.is_empty()) {
        return;
    }

    const SDL_Rect square = board.square_bounds(file, rank);
    if (square.w <= 0 || square.h <= 0) {
        return;
    }

    const int center_x = square.x + square.w / 2;
    const int center_y = square.y + square.h / 2;
    const int radius = static_cast<int>(min(square.w, square.h) * 0.40f);

    if (piece.color == Color::White) {
        SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
        fill_circle(renderer, center_x, center_y, radius);
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        stroke_circle(renderer, center_x, center_y, radius);
    } else {
        SDL_SetRenderDrawColor(renderer, 25, 25, 25, 255);
        fill_circle(renderer, center_x, center_y, radius);
        SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
        stroke_circle(renderer, center_x, center_y, radius);
    }

    const char* filename = piece_asset_file(piece.type);
    SDL_Texture* icon = filename ? assets.piece(filename) : nullptr;
    const int icon_size = static_cast<int>(radius * 2 * ICON_SCALE);

    if (icon) {
        draw_centered_texture(renderer, icon, square, icon_size);
    } else {
        draw_fallback_label(renderer, square, piece.color, piece.type);
    }
}

void PieceRenderer::draw_starting_position(
    SDL_Renderer* renderer,
    const AssetLoader& assets,
    const BoardRenderer& board
) {
    const PieceType back_rank[] = {
        PieceType::Rook,
        PieceType::Knight,
        PieceType::Bishop,
        PieceType::Queen,
        PieceType::King,
        PieceType::Bishop,
        PieceType::Knight,
        PieceType::Rook,
    };

    for (int file = 0; file < 8; ++file) {
        draw(renderer, assets, board, {back_rank[file], Color::White}, file, 0);
        draw(renderer, assets, board, {PieceType::Pawn, Color::White}, file, 1);
        draw(renderer, assets, board, {PieceType::Pawn, Color::Black}, file, 6);
        draw(renderer, assets, board, {back_rank[file], Color::Black}, file, 7);
    }
}
