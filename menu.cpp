#include "menu.h"
#include "texture_manager.h"
#include "game.h" // Để truy cập enemyTanks và resetPlayerPosition
#include <iostream>

std::vector<Button> buttons = {
    {{300, 340, 200, 50}, nullptr},
    {{300, 410, 200, 50}, nullptr},
    {{300, 480, 200, 50}, nullptr}
};

void renderMenu(SDL_Renderer* renderer) {
    SDL_RenderCopy(renderer, TextureManager::getTexture("image/background.png", renderer), nullptr, nullptr);
    for (const auto& button : buttons) {
        SDL_RenderCopy(renderer, button.texture, nullptr, &button.rect);
    }
}

void handleMenuInput(const SDL_Event& e, GameState* gameState, SDL_Renderer* renderer) {
    if (e.type == SDL_MOUSEBUTTONDOWN) {
        int x, y;
        SDL_GetMouseState(&x, &y);

        for (size_t i = 0; i < buttons.size(); ++i) {
            if (x >= buttons[i].rect.x && x <= buttons[i].rect.x + buttons[i].rect.w &&
                y >= buttons[i].rect.y && y <= buttons[i].rect.y + buttons[i].rect.h) {
                if (i == 0) {
                    *gameState = GAME;
                    enemyTanks.clear();
                    TextureManager::gTextureCache["image/Tank.png"] = TextureManager::loadTexture("image/Tank.png", renderer);
                    TextureManager::gTextureCache["image/Tank1.png"] = TextureManager::loadTexture("image/Tank1.png", renderer);
                    int numEnemies = 4;
                    for (int i = 0; i < numEnemies; i++) {
                        float enemyX, enemyY;
                        enemyX = (float)40 * (20 - 2 - i * 3);
                        enemyY = (float)40 * (1 + i * 2);
                        Tank enemyTank1 = {enemyX, enemyY, NONE, 0, nullptr, DOWN, enemyX, enemyY, false, 0, 0, true};
                        enemyTank1.texture = TextureManager::getTexture("image/Tank1.png", renderer);
                        enemyTanks.push_back(enemyTank1);
                    }
                    resetPlayerPosition(); // Đặt lại vị trí xe tăng người chơi khi bắt đầu game
                    playerLives = 2;
                    playerTank.direction = NONE;
                    playerTank.lastValidDirection = DOWN;
                    playerTank.active = true;
                }
            }
        }
    }
}
