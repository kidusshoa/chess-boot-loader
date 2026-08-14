#pragma once

#include <SDL2/SDL.h>

#include <string>
#include <unordered_map>

class AssetLoader {
public:
    explicit AssetLoader(SDL_Renderer* renderer);
    ~AssetLoader();

    AssetLoader(const AssetLoader&) = delete;
    AssetLoader& operator=(const AssetLoader&) = delete;

    bool load_board();
    bool load_piece(const std::string& filename);

    SDL_Texture* board() const;
    SDL_Texture* piece(const std::string& filename) const;

private:
    SDL_Renderer* renderer_;
    SDL_Texture* board_texture_;
    std::unordered_map<std::string, SDL_Texture*> piece_textures_;
    bool image_initialized_;

    SDL_Surface* load_surface_from_file(const std::string& path);
    SDL_Texture* load_texture_from_file(const std::string& path, bool piece_texture = false);
    void free_texture(SDL_Texture*& texture);
};
