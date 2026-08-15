#pragma once

#include <SDL2/SDL.h>

// Rasterize an SVG file to an SDL surface. Returns nullptr on failure.
SDL_Surface* load_svg_surface(const char* path, int width, int height);
