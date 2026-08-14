#pragma once

#include <SDL2/SDL.h>

using BootTransitionDrawFn = void (*)(SDL_Renderer* renderer, void* context);

// Runs the BIOS-style boot sequence. Returns false if the window was closed.
bool run_boot_splash(SDL_Renderer* renderer, SDL_Window* window);

// Fades from black into the first game frame drawn by draw_frame.
bool run_game_fade_in(
    SDL_Renderer* renderer,
    SDL_Window* window,
    BootTransitionDrawFn draw_frame,
    void* context
);

bool should_skip_boot(int argc, char* argv[]);
