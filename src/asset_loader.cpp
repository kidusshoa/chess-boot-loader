#include "asset_loader.h"

#include <iostream>
using namespace std;

#include <SDL2/SDL_image.h>

#include <cstring>

namespace {

constexpr const char* BOARD_PATH = "assets/chess-board-banner-vector.jpg";
constexpr const char* PIECES_DIR = "assets/pieces/";
constexpr int SVG_RASTER_SIZE = 256;

bool ends_with(const string& value, const char* suffix) {
    const size_t suffix_length = strlen(suffix);
    if (value.size() < suffix_length) {
        return false;
    }
    return value.compare(value.size() - suffix_length, suffix_length, suffix) == 0;
}

}  // namespace

AssetLoader::AssetLoader(SDL_Renderer* renderer)
    : renderer_(renderer),
      board_texture_(nullptr),
      image_initialized_(false) {
    int flags = IMG_INIT_JPG | IMG_INIT_PNG;
#ifdef IMG_INIT_SVG
    flags |= IMG_INIT_SVG;
#endif

    const int initialized = IMG_Init(flags);
    if ((initialized & (IMG_INIT_JPG | IMG_INIT_PNG)) != (IMG_INIT_JPG | IMG_INIT_PNG)) {
        cerr << "IMG_Init failed: " << IMG_GetError() << "\n";
        return;
    }

    image_initialized_ = true;
}

AssetLoader::~AssetLoader() {
    free_texture(board_texture_);

    for (auto& entry : piece_textures_) {
        free_texture(entry.second);
    }
    piece_textures_.clear();

    if (image_initialized_) {
        IMG_Quit();
    }
}

bool AssetLoader::load_board() {
    if (!image_initialized_) {
        cerr << "Cannot load board: SDL2_image is not initialized\n";
        return false;
    }

    if (board_texture_) {
        return true;
    }

    board_texture_ = load_texture_from_file(BOARD_PATH);
    if (!board_texture_) {
        cerr << "Failed to load board asset: " << BOARD_PATH << "\n";
        return false;
    }

    return true;
}

bool AssetLoader::load_piece(const string& filename) {
    if (!image_initialized_) {
        cerr << "Cannot load piece: SDL2_image is not initialized\n";
        return false;
    }

    if (piece_textures_.count(filename) > 0) {
        return true;
    }

    const string path = string(PIECES_DIR) + filename;
    SDL_Texture* texture = load_texture_from_file(path, true);
    if (!texture) {
        cerr << "Failed to load piece asset: " << path << "\n";
        return false;
    }

    piece_textures_[filename] = texture;
    return true;
}

SDL_Texture* AssetLoader::board() const {
    return board_texture_;
}

SDL_Texture* AssetLoader::piece(const string& filename) const {
    const auto it = piece_textures_.find(filename);
    if (it == piece_textures_.end()) {
        return nullptr;
    }
    return it->second;
}

SDL_Texture* AssetLoader::load_texture_from_file(const string& path, bool piece_texture) {
    SDL_Surface* surface = load_surface_from_file(path);
    if (!surface) {
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
    SDL_FreeSurface(surface);

    if (!texture) {
        cerr << "SDL_CreateTextureFromSurface failed for " << path << ": " << SDL_GetError() << "\n";
        return nullptr;
    }

    if (piece_texture) {
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    }

    return texture;
}

SDL_Surface* AssetLoader::load_surface_from_file(const string& path) {
    SDL_Surface* surface = nullptr;

    if (ends_with(path, ".svg")) {
#if SDL_IMAGE_VERSION_ATLEAST(2, 6, 0)
        surface = IMG_LoadSizedSVG(path.c_str(), SVG_RASTER_SIZE, SVG_RASTER_SIZE);
        if (!surface) {
            cerr << "IMG_LoadSizedSVG failed for " << path << ": " << IMG_GetError() << "\n";
        }
#endif
        if (!surface) {
            surface = IMG_Load(path.c_str());
        }
    } else {
        surface = IMG_Load(path.c_str());
    }

    if (!surface) {
        cerr << "IMG_Load failed for " << path << ": " << IMG_GetError() << "\n";
        return nullptr;
    }

    return surface;
}

void AssetLoader::free_texture(SDL_Texture*& texture) {
    if (texture) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
}
