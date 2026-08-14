#include "asset_loader.h"
#include "board_renderer.h"
#include "boot_splash.h"
#include "chess_board.h"
#include "game_controller.h"
#include "game_ui.h"
#include "piece_renderer.h"

#include <iostream>
using namespace std;

#include <SDL2/SDL.h>

#include <string>

struct GameDrawContext {
    SDL_Texture* board_texture;
    int texture_width;
    int texture_height;
    AssetLoader* assets;
    ChessBoard* chess_board;
    BoardRenderer* board_renderer;
    GameController* game;
};

bool parse_debug_flag(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        if (string(argv[i]) == "--debug") {
            return true;
        }
    }
    return false;
}

void draw_game_frame(SDL_Renderer* renderer, void* context) {
    auto* ctx = static_cast<GameDrawContext*>(context);

    int width = 0;
    int height = 0;
    SDL_GetRendererOutputSize(renderer, &width, &height);
    ctx->board_renderer->update_layout(width, height, ctx->texture_width, ctx->texture_height);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    ctx->board_renderer->draw(renderer, ctx->board_texture);
    ctx->chess_board->draw(renderer, *ctx->assets, *ctx->board_renderer);
    ctx->game->draw_highlights(renderer, *ctx->chess_board, *ctx->board_renderer);
    draw_game_ui(renderer, *ctx->game, width, height);
    ctx->board_renderer->draw_debug_overlay(renderer);
}

void handle_mouse_click(
    SDL_Renderer* renderer,
    SDL_Window* window,
    const SDL_MouseButtonEvent& button,
    GameController& game,
    ChessBoard& chess_board,
    const BoardRenderer& board_renderer
) {
    int window_width = 0;
    int window_height = 0;
    int output_width = 0;
    int output_height = 0;
    SDL_GetWindowSize(window, &window_width, &window_height);
    SDL_GetRendererOutputSize(renderer, &output_width, &output_height);

    if (window_width <= 0 || window_height <= 0) {
        return;
    }

    const float scale_x = output_width / static_cast<float>(window_width);
    const float scale_y = output_height / static_cast<float>(window_height);
    const int x = static_cast<int>(button.x * scale_x);
    const int y = static_cast<int>(button.y * scale_y);

    game.handle_click(chess_board, board_renderer, x, y);
}

int main(int argc, char* argv[]) {
    const bool debug_overlay = parse_debug_flag(argc, argv);
    const bool skip_boot = should_skip_boot(argc, argv);

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

    if (!skip_boot) {
        if (!run_boot_splash(renderer, window)) {
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            SDL_Quit();
            return 0;
        }
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
    GameController game;

    GameDrawContext draw_context = {
        board_texture,
        texture_width,
        texture_height,
        &assets,
        &chess_board,
        &board_renderer,
        &game,
    };

    if (!run_game_fade_in(renderer, window, draw_game_frame, &draw_context)) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                handle_mouse_click(renderer, window, event.button, game, chess_board, board_renderer);
            } else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_r) {
                game.restart(chess_board);
            }
        }

        draw_game_frame(renderer, &draw_context);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    cout << "chess-boot-loader\n";
    return 0;
}
