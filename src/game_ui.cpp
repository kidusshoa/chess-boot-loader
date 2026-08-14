#include "game_ui.h"

#include "game_controller.h"

#include "chess_types.h"

#include <SDL2/SDL_ttf.h>

namespace {

TTF_Font* ui_font() {
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
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
    };

    for (const char* path : font_paths) {
        font = TTF_OpenFont(path, 20);
        if (font) {
            break;
        }
    }

    return font;
}

void draw_text(SDL_Renderer* renderer, TTF_Font* font, const char* text, int x, int y, SDL_Color color) {
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

void draw_bar(SDL_Renderer* renderer, int window_width, int height, int y) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_Rect bar = {0, y, window_width, height};
    SDL_RenderFillRect(renderer, &bar);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

const char* status_message(const GameController& game) {
    switch (game.game_result()) {
        case GameResult::WhiteWins:
            return "Checkmate - White wins";
        case GameResult::BlackWins:
            return "Checkmate - Black wins";
        case GameResult::Draw:
            return "Stalemate - Draw";
        case GameResult::InProgress:
        default:
            break;
    }

    if (game.is_in_check()) {
        return game.current_player() == Color::White ? "White to move - Check!" : "Black to move - Check!";
    }

    return game.current_player() == Color::White ? "White to move" : "Black to move";
}

}  // namespace

void draw_game_ui(SDL_Renderer* renderer, const GameController& game, int window_width, int window_height) {
    TTF_Font* font = ui_font();
    if (!font) {
        return;
    }

    const SDL_Color white = {240, 240, 240, 255};
    const SDL_Color amber = {255, 191, 64, 255};
    const SDL_Color green = {96, 220, 120, 255};

    draw_bar(renderer, window_width, 40, 0);
    draw_text(renderer, font, status_message(game), 16, 10, white);

    if (game.game_result() != GameResult::InProgress) {
        draw_bar(renderer, window_width, window_height, 0);

        const char* headline = status_message(game);
        SDL_Surface* headline_surface = TTF_RenderText_Blended(font, headline, amber);
        if (!headline_surface) {
            return;
        }

        SDL_Texture* headline_texture = SDL_CreateTextureFromSurface(renderer, headline_surface);
        const int headline_width = headline_surface->w;
        const int headline_height = headline_surface->h;
        SDL_FreeSurface(headline_surface);

        if (headline_texture) {
            const SDL_Rect headline_dest = {
                (window_width - headline_width) / 2,
                window_height / 2 - 40,
                headline_width,
                headline_height,
            };
            SDL_RenderCopy(renderer, headline_texture, nullptr, &headline_dest);
            SDL_DestroyTexture(headline_texture);
        }

        draw_text(
            renderer,
            font,
            "Press R to restart",
            (window_width - 180) / 2,
            window_height / 2 + 10,
            green
        );
    }
}
