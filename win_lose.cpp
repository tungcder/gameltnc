#include "win_lose.h"
#include "texture_manager.h"

void renderWin(SDL_Renderer* renderer) {
    SDL_RenderCopy(renderer, TextureManager::getTexture("image/win.png", renderer), nullptr, nullptr);
}

void renderLose(SDL_Renderer* renderer) {
    SDL_RenderCopy(renderer, TextureManager::getTexture("image/lose.png", renderer), nullptr, nullptr);
}
