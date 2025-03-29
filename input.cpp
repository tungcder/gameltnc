#include "input.h"
#include "game.h"
#include "graphics.h"
#include <SDL_mixer.h>

void handleMenuInput(const SDL_Event& e) {
    if (e.type == SDL_MOUSEBUTTONDOWN) {
        int x, y;
        SDL_GetMouseState(&x, &y);

        for (size_t i = 0; i < buttons.size(); ++i) {
            if (x >= buttons[i].rect.x && x <= buttons[i].rect.x + buttons[i].rect.w &&
                y >= buttons[i].rect.y && y <= buttons[i].rect.y + buttons[i].rect.h) {
                if (i == 0) {
                    gameState = GAME;
                    enemyTanks.clear();
                    gTextureCache["image/Tank.png"] = loadTexture("image/Tank.png");
                    gTextureCache["image/Tank1.png"] = loadTexture("image/Tank1.png");
                    int numEnemies = 4;
                    for (int i = 0; i < numEnemies; i++) {
                        float enemyX, enemyY;
                        enemyX = (float)TILE_SIZE * (MAP_COLS - 2 - i * 3);
                        enemyY = (float)TILE_SIZE * (1 + i * 2);
                        Tank enemyTank1 = {enemyX, enemyY, NONE, 0, nullptr, DOWN, enemyX, enemyY, false, 0, 0, true};
                        enemyTank1.texture = getTexture("image/Tank1.png");
                        enemyTanks.push_back(enemyTank1);
                    }
                    playerTank.x = (float)TILE_SIZE;
                    playerTank.y = (float)TILE_SIZE;
                    playerTank.direction = NONE;
                    playerTank.lastValidDirection = DOWN;
                    playerTank.active = true;
                    Mix_HaltMusic();
                    Mix_Music* gameStartMusic = Mix_LoadMUS("sound/intro1.wav");
                    if (gameStartMusic) {
                        Mix_PlayMusic(gameStartMusic, -1);
                        Mix_FreeMusic(gameStartMusic);
                    }
                    else {
                        std::cout << "Failed to load game start music! SDL_mixer Error: " << Mix_GetError() << std::endl;
                    }
                }
            }
        }
    }
}

void handleGameInput(const SDL_Event& e) {
    const Uint8 *keystate = SDL_GetKeyboardState(NULL);
    if (e.type == SDL_KEYDOWN) {
        switch (e.key.keysym.sym) {
            case SDLK_UP:
                playerTank.direction = UP;
                playerTank.lastValidDirection = UP;
                break;
            case SDLK_DOWN:
                playerTank.direction = DOWN;
                playerTank.lastValidDirection = DOWN;
                break;
            case SDLK_LEFT:
                playerTank.direction = LEFT;
                playerTank.lastValidDirection = LEFT;
                break;
            case SDLK_RIGHT:
                playerTank.direction = RIGHT;
                playerTank.lastValidDirection = RIGHT;
                break;
            default: break;
        }
    }
    else if (e.type == SDL_KEYUP) {
        switch (e.key.keysym.sym) {
            case SDLK_UP:
            case SDLK_DOWN:
            case SDLK_LEFT:
            case SDLK_RIGHT:
                if (!(keystate[SDL_SCANCODE_UP] ||
                      keystate[SDL_SCANCODE_DOWN] ||
                      keystate[SDL_SCANCODE_LEFT] ||
                      keystate[SDL_SCANCODE_RIGHT])) {
                    playerTank.direction = NONE;
                }
                break;
            default: break;
        }
    }
    if (keystate[SDL_SCANCODE_SPACE]) {
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - lastBulletTime > BULLET_COOLDOWN) {
            createBullet(playerTank);
            lastBulletTime = currentTime;
        }
    }
}
