#pragma once

#include <SDL2/SDL.h>

// Runs the BIOS-style boot sequence. Returns false if the window was closed.
bool run_boot_splash(SDL_Renderer* renderer, SDL_Window* window);
