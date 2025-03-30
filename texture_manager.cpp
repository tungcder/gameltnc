#include "texture_manager.h"
#include <iostream>

std::unordered_map<std::string, SDL_Texture*> TextureManager::gTextureCache;

SDL_Texture* TextureManager::loadTexture(const std::string& path, SDL_Renderer* renderer) {
    SDL_Texture* newTexture = nullptr;
    SDL_Surface* loadedSurface = IMG_Load(path.c_str());
    if (!loadedSurface) {
        std::cout << "Unable to load image " << path << "! SDL_image Error: " << IMG_GetError() << std::endl;
        return nullptr;
    } else {
        newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
        SDL_FreeSurface(loadedSurface);
    }
    return newTexture;
}

SDL_Texture* TextureManager::getTexture(const std::string& path, SDL_Renderer* renderer) {
    if (gTextureCache.find(path) != gTextureCache.end()) {
        return gTextureCache[path];
    } else {
        SDL_Texture* texture = loadTexture(path, renderer);
        gTextureCache[path] = texture;
        return texture;
    }
}

void TextureManager::clearCache() {
    for (auto const& [key, val] : gTextureCache) {
        SDL_DestroyTexture(val);
    }
    gTextureCache.clear();
}
