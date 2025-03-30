#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

#include <SDL.h>
#include <SDL_image.h>
#include <string>
#include <unordered_map>

// Forward declaration
class TextureManager {
public:
    static SDL_Texture* loadTexture(const std::string& path, SDL_Renderer* renderer);
    static SDL_Texture* getTexture(const std::string& path, SDL_Renderer* renderer);
    static void clearCache(); // Thêm hàm này
private:
    static std::unordered_map<std::string, SDL_Texture*> gTextureCache;
};

#endif
