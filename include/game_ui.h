#pragma once

class GameController;

#include <SDL2/SDL.h>

void draw_game_ui(SDL_Renderer* renderer, const GameController& game, int window_width, int window_height);
