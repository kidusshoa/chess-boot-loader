#include "boot_splash.h"
#include "version.h"

#include <iostream>
using namespace std;

#include <SDL2/SDL_ttf.h>

#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

namespace {

constexpr int TYPEWRITER_DELAY_MS = 24;
constexpr int AUTO_TRANSITION_MS = 3800;
constexpr int BOOT_FADE_OUT_MS = 350;
constexpr int GAME_FADE_IN_MS = 700;
constexpr int LINE_HEIGHT = 24;
constexpr int MARGIN_X = 32;
constexpr int MARGIN_Y = 36;
constexpr int FONT_SIZE = 20;

vector<string> boot_lines() {
    return {
        string("chess-boot-loader v") + CHESS_BOOT_LOADER_VERSION,
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
}

TTF_Font* load_font() {
    const char* font_paths[] = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
        "/System/Library/Fonts/Menlo.ttc",
        "/Library/Fonts/Menlo.ttc",
    };

    for (const char* path : font_paths) {
        TTF_Font* font = TTF_OpenFont(path, FONT_SIZE);
        if (font) {
            return font;
        }
    }

    return nullptr;
}

void render_line(SDL_Renderer* renderer, TTF_Font* font, const char* text, int x, int y, Uint8 alpha) {
    if (!text || text[0] == '\0') {
        return;
    }

    SDL_Color color = {0, 255, 0, alpha};
    SDL_Surface* surface = TTF_RenderText_Blended(font, text, color);
    if (!surface) {
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) {
        return;
    }

    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureAlphaMod(texture, alpha);

    SDL_Rect dest = {x, y, 0, 0};
    SDL_QueryTexture(texture, nullptr, nullptr, &dest.w, &dest.h);
    SDL_RenderCopy(renderer, texture, nullptr, &dest);
    SDL_DestroyTexture(texture);
}

void render_boot_frame(
    SDL_Renderer* renderer,
    TTF_Font* font,
    const vector<string>& lines,
    size_t line_index,
    size_t char_index,
    Uint8 text_alpha
) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    int y = MARGIN_Y;
    for (size_t i = 0; i < line_index; ++i) {
        render_line(renderer, font, lines[i].c_str(), MARGIN_X, y, text_alpha);
        y += LINE_HEIGHT;
    }

    if (line_index < lines.size()) {
        string partial;
        const string& current_line = lines[line_index];
        if (char_index > 0) {
            partial.assign(current_line, 0, char_index);
        }
        render_line(renderer, font, partial.c_str(), MARGIN_X, y, text_alpha);
    }
}

bool poll_close_event() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return true;
        }
    }
    return false;
}

void draw_fade_overlay(SDL_Renderer* renderer, int window_width, int window_height, Uint8 alpha) {
    if (alpha == 0) {
        return;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, alpha);
    SDL_Rect overlay = {0, 0, window_width, window_height};
    SDL_RenderFillRect(renderer, &overlay);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

bool fade_out_boot_screen(
    SDL_Renderer* renderer,
    SDL_Window* window,
    TTF_Font* font,
    const vector<string>& lines,
    size_t line_index,
    size_t char_index
) {
    const Uint32 fade_start = SDL_GetTicks();

    while (true) {
        if (poll_close_event()) {
            return false;
        }

        const Uint32 elapsed = SDL_GetTicks() - fade_start;
        if (elapsed >= static_cast<Uint32>(BOOT_FADE_OUT_MS)) {
            break;
        }

        const float progress = elapsed / static_cast<float>(BOOT_FADE_OUT_MS);
        const Uint8 text_alpha = static_cast<Uint8>(255.0f * (1.0f - progress));

        render_boot_frame(renderer, font, lines, line_index, char_index, text_alpha);

        int width = 0;
        int height = 0;
        SDL_GetRendererOutputSize(renderer, &width, &height);
        draw_fade_overlay(renderer, width, height, static_cast<Uint8>(255.0f * progress));

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    (void)window;
    return true;
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

    const vector<string> lines = boot_lines();
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

            if (line_index < lines.size()) {
                const size_t line_length = lines[line_index].size();
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

        render_boot_frame(renderer, font, lines, line_index, char_index, 255);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);

        if (sequence_complete && now - start_ticks >= static_cast<Uint32>(AUTO_TRANSITION_MS)) {
            skip = true;
        }
    }

    const bool fade_ok = fade_out_boot_screen(renderer, window, font, lines, line_index, char_index);

    TTF_CloseFont(font);
    TTF_Quit();
    return fade_ok;
}

bool run_game_fade_in(
    SDL_Renderer* renderer,
    SDL_Window* window,
    BootTransitionDrawFn draw_frame,
    void* context
) {
    if (!draw_frame) {
        return false;
    }

    const Uint32 fade_start = SDL_GetTicks();

    while (true) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return false;
            }
        }

        int width = 0;
        int height = 0;
        SDL_GetRendererOutputSize(renderer, &width, &height);

        draw_frame(renderer, context);

        const Uint32 elapsed = SDL_GetTicks() - fade_start;
        if (elapsed >= static_cast<Uint32>(GAME_FADE_IN_MS)) {
            SDL_RenderPresent(renderer);
            break;
        }

        const float progress = elapsed / static_cast<float>(GAME_FADE_IN_MS);
        const Uint8 overlay_alpha = static_cast<Uint8>(255.0f * (1.0f - progress));
        draw_fade_overlay(renderer, width, height, overlay_alpha);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    (void)window;
    return true;
}
