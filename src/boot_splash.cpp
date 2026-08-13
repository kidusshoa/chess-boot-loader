#include "boot_splash.h"

#include <iostream>
using namespace std;

#include <SDL2/SDL_ttf.h>

#include <cstring>
#include <string>
#include <vector>

namespace {

constexpr int TYPEWRITER_DELAY_MS = 28;
constexpr int AUTO_TRANSITION_MS = 4000;
constexpr int LINE_HEIGHT = 22;
constexpr int MARGIN_X = 32;
constexpr int MARGIN_Y = 32;

const vector<const char*> BOOT_LINES = {
    "chess-boot-loader v0.1.0",
    "",
    "POST: Memory check ............... OK",
    "POST: CPU detected ............... OK",
    "Loading chess engine...",
    "Initializing C king...",
    "Initializing Java queen...",
    "Initializing Python bishop...",
    "Initializing JavaScript knight...",
    "Initializing Rust rook...",
    "Initializing Go pawn...",
    "",
    "Boot complete. Starting game...",
};

TTF_Font* load_font() {
    const char* font_paths[] = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
        "/System/Library/Fonts/Menlo.ttc",
        "/Library/Fonts/Menlo.ttc",
    };

    for (const char* path : font_paths) {
        TTF_Font* font = TTF_OpenFont(path, 18);
        if (font) {
            return font;
        }
    }

    return nullptr;
}

void render_line(SDL_Renderer* renderer, TTF_Font* font, const char* text, int x, int y) {
    if (!text || text[0] == '\0') {
        return;
    }

    SDL_Color color = {0, 255, 0, 255};
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

bool run_boot_splash(SDL_Renderer* renderer, SDL_Window* window) {
    if (TTF_Init() != 0) {
        cerr << "TTF_Init failed: " << TTF_GetError() << "\n";
        return false;
    }

    TTF_Font* font = load_font();
    if (!font) {
        cerr << "Failed to load monospace font for boot splash\n";
        TTF_Quit();
        return false;
    }

    const Uint32 start_ticks = SDL_GetTicks();
    Uint32 last_char_ticks = start_ticks;

    size_t line_index = 0;
    size_t char_index = 0;
    bool sequence_complete = false;
    bool skip = false;

    while (!skip) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                TTF_CloseFont(font);
                TTF_Quit();
                return false;
            }
            if (event.type == SDL_KEYDOWN || event.type == SDL_MOUSEBUTTONDOWN) {
                skip = true;
            }
        }

        const Uint32 now = SDL_GetTicks();
        if (now - start_ticks >= static_cast<Uint32>(AUTO_TRANSITION_MS)) {
            skip = true;
        }

        if (!sequence_complete && now - last_char_ticks >= static_cast<Uint32>(TYPEWRITER_DELAY_MS)) {
            last_char_ticks = now;

            if (line_index < BOOT_LINES.size()) {
                const size_t line_length = BOOT_LINES[line_index] ? strlen(BOOT_LINES[line_index]) : 0;
                if (char_index < line_length) {
                    ++char_index;
                } else {
                    ++line_index;
                    char_index = 0;
                }
            } else {
                sequence_complete = true;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        int y = MARGIN_Y;
        for (size_t i = 0; i < line_index; ++i) {
            render_line(renderer, font, BOOT_LINES[i], MARGIN_X, y);
            y += LINE_HEIGHT;
        }

        if (line_index < BOOT_LINES.size()) {
            string partial;
            const char* current_line = BOOT_LINES[line_index];
            if (current_line && char_index > 0) {
                partial.assign(current_line, char_index);
            }
            render_line(renderer, font, partial.c_str(), MARGIN_X, y);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);

        if (sequence_complete && now - start_ticks >= static_cast<Uint32>(AUTO_TRANSITION_MS)) {
            skip = true;
        }
    }

    (void)window;

    TTF_CloseFont(font);
    TTF_Quit();
    return true;
}
