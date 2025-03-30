#include "utils.h"

SDL_Color gTextColor = { 0, 0, 0 }; // Màu chữ trắng

void renderLives(SDL_Renderer* renderer, int lives, TTF_Font* gFont) {
    std::stringstream ss;
    ss << "Lives: " << lives;
    std::string livesText = ss.str();

    SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, livesText.c_str(), gTextColor);
    if (textSurface == nullptr) {
        std::cout << "Unable to render text surface! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return;
    }

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (textTexture == nullptr) {
        std::cout << "Unable to create texture from rendered text! SDL Error: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(textSurface);
        return;
    }

    SDL_Rect textRect;
    textRect.x = 10;
    textRect.y = 10;
    textRect.w = textSurface->w;
    textRect.h = textSurface->h;

    SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}
