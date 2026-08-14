#include "asset_loader.h"
#include "board_renderer.h"
#include "boot_splash.h"
#include "chess_board.h"
#include "piece_renderer.h"

#include <iostream>
using namespace std;

#include <SDL2/SDL.h>

#include <string>

bool parse_debug_flag(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        if (string(argv[i]) == "--debug") {
            return true;
        }
    }
    return false;
}

int main(int argc, char* argv[]) {
    const bool debug_overlay = parse_debug_flag(argc, argv);

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "chess-boot-loader",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800,
        800,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    if (!window) {
        cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    if (!run_boot_splash(renderer, window)) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }

    AssetLoader assets(renderer);
    if (!assets.load_board()) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Texture* board_texture = assets.board();
    int texture_width = 0;
    int texture_height = 0;
    SDL_QueryTexture(board_texture, nullptr, nullptr, &texture_width, &texture_height);

    BoardRenderer board_renderer(debug_overlay);

    PieceRenderer::load_standard_pieces(assets);

    ChessBoard chess_board;

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        int width = 0;
        int height = 0;
        SDL_GetRendererOutputSize(renderer, &width, &height);
        board_renderer.update_layout(width, height, texture_width, texture_height);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        board_renderer.draw(renderer, board_texture);
        chess_board.draw(renderer, assets, board_renderer);
        board_renderer.draw_debug_overlay(renderer);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    cout << "chess-boot-loader\n";
    return 0;
}
