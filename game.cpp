#include "game.h"
#include "constants.h"
#include "map.h"
#include "texture_manager.h"
#include <algorithm>
#include <iostream>

std::vector<Explosion> explosions;
std::vector<Tank> enemyTanks;
std::vector<Bullet> bullets;

Tank playerTank = {
    (float)40, (float)40,
    NONE,
    0,
    nullptr,
    DOWN,
    (float)40, (float)40,
    true,
    0,
    0,
    true
};
int playerLives = 2;
Uint32 lastBulletTime = 0;

bool checkCollision(float x1, float y1, int w1, int h1, float x2, float y2, int w2, int h2) {
    return (x1 < x2 + w2 &&
            x1 + w1 > x2 &&
            y1 < y2 + h2 &&
            y1 + h1 > y2);
}

bool isValidMove(float x, float y, int width, int height, Tank* currentTank) {
    int topLeftX = (int)floor(x / 40);
    int topLeftY = (int)floor(y / 40);
    int topRightX = (int)floor((x + width - 1) / 40);
    int topRightY = (int)floor(y / 40);
    int bottomLeftX = (int)floor(x / 40);
    int bottomLeftY = (int)floor((y + height - 1) / 40);
    int bottomRightX = (int)floor((x + width - 1) / 40);
    int bottomRightY = (int)floor((y + height - 1) / 40);

    if (isTileBlocked(topLeftY, topLeftX) ||
        isTileBlocked(topRightY, topRightX) ||
        isTileBlocked(bottomLeftY, bottomLeftX) ||
        isTileBlocked(bottomRightY, bottomRightX)) {
        return false;
    }

    // Kiểm tra va chạm với xe tăng khác
    if (currentTank->isPlayer) {
        for (auto& enemyTank : enemyTanks) {
            if (!enemyTank.active) continue;
            if (&enemyTank == currentTank) continue; // Bỏ qua kiểm tra với chính nó

            if (checkCollision(x, y, width, height, enemyTank.x, enemyTank.y, 40, 40)) {
                return false;
            }
        }
    } else {
        if (playerTank.active && checkCollision(x, y, width, height, playerTank.x, playerTank.y, 40, 40)) {
            return false;
        }
        for (auto& enemyTank : enemyTanks) {
            if (!enemyTank.active) continue;
            if (&enemyTank == currentTank) continue; // Bỏ qua kiểm tra với chính nó

            if (checkCollision(x, y, width, height, enemyTank.x, enemyTank.y, 40, 40)) {
                return false;
            }
        }
    }
    return true;
}

void createBullet(Tank& tank, SDL_Renderer* renderer) {
    Bullet newBullet;
    newBullet.x = tank.x + 40 / 2 - 10 / 2;
    newBullet.y = tank.y + 40 / 2 - 10 / 2;
    newBullet.direction = (tank.direction == NONE) ? tank.lastValidDirection : tank.direction;
    newBullet.texture = TextureManager::getTexture("image/bullet.png", renderer);
    newBullet.active = true;
    newBullet.isPlayerBullet = tank.isPlayer;

    bullets.push_back(newBullet);
}

void updateBullets(SDL_Renderer* renderer) {
    for (auto it = bullets.begin(); it != bullets.end();) {
        Bullet& bullet = *it;
        if (!bullet.active) {
            it = bullets.erase(it);
            continue;
        }
        switch (bullet.direction) {
            case UP: bullet.y -= 6.0f; break;
            case DOWN: bullet.y += 6.0f; break;
            case LEFT: bullet.x -= 6.0f; break;
            case RIGHT: bullet.x += 6.0f; break;
            case NONE: bullet.active = false; break;
        }
        if (bullet.x < 0 || bullet.x > 800 || bullet.y < 0 || bullet.y > 600) {
            bullet.active = false;
        }
        int tileX = (int)(bullet.x / 40);
        int tileY = (int)(bullet.y / 40);

        if (tileX >= 0 && tileX < 20 && tileY >= 0 && tileY < 15) {
            if (map[tileY][tileX] == 1) {
                map[tileY][tileX] = 0;
                bullet.active = false;
                createExplosion(tileX * 40, tileY * 40);
                //Mix_PlayChannel(-1, gBulletHitSound, 0);

            } else if (map[tileY][tileX] != 0 && map[tileY][tileX] != 4) {
                bullet.active = false;
                createExplosion(bullet.x - 10 / 2, bullet.y - 10 / 2);
                //Mix_PlayChannel(-1, gBulletHitSound, 0);
            }
        }
        if (bullet.isPlayerBullet) {
            for (auto& enemyTank : enemyTanks) {
                if (!enemyTank.active) continue;
                if (checkCollision(bullet.x, bullet.y, 10, 10, enemyTank.x, enemyTank.y, 40, 40)) {
                    bullet.active = false;
                    createExplosion(enemyTank.x, enemyTank.y);
                    enemyTank.active = false;
                    //Mix_PlayChannel(-1, gBulletHitSound, 0);
                    break;
                }
            }
        } else {
            if (playerTank.active && checkCollision(bullet.x, bullet.y, 10, 10, playerTank.x, playerTank.y, 40, 40)) {
                bullet.active = false;
                createExplosion(playerTank.x, playerTank.y);
                playerTank.active = false;
                playerLives--; // Giảm số mạng khi xe tăng người chơi bị bắn
                if (playerLives > 0) {
                    resetPlayerPosition(); // Đặt lại vị trí nếu còn mạng
                    playerTank.active = true;
                }
               // Mix_PlayChannel(-1, gBulletHitSound, 0);
            }
        }
         ++it;
    }
    bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [](const Bullet& b) { return !b.active; }), bullets.end());
    enemyTanks.erase(std::remove_if(enemyTanks.begin(), enemyTanks.end(), [](const Tank& t) { return !t.active; }), enemyTanks.end());
}

void createExplosion(float x, float y) {
    Explosion newExplosion;
    newExplosion.x = x;
    newExplosion.y = y;
    newExplosion.frame = 0;
    newExplosion.frameTimer = 0;
    newExplosion.active = true;
    explosions.push_back(newExplosion);
}

void updateExplosions() {
    for (auto& explosion : explosions) {
        if (explosion.active) {
            explosion.frameTimer++;

            if (explosion.frameTimer >= 5) {
                explosion.frameTimer = 0;
                explosion.frame++;

                if (explosion.frame >= 3) {
                    explosion.active = false;
                }
            }
        }
    }
    explosions.erase(std::remove_if(explosions.begin(), explosions.end(), [](const Explosion& e) { return !e.active; }), explosions.end());
}

Direction getRandomDirection() {
    int randDir = rand() % 4;
    switch (randDir) {
        case 0: return UP;
        case 1: return DOWN;
        case 2: return LEFT;
        case 3: return RIGHT;
        default: return NONE;
    }
}

void moveAITank(Tank& tank) {
    float newX = tank.x;
    float newY = tank.y;

    switch (tank.direction) {
        case UP:    newY -= 2.0f; break;
        case DOWN:  newY += 2.0f; break;
        case LEFT:  newX -= 2.0f; break;
        case RIGHT: newX += 2.0f; break;
        case NONE:  break;
    }

    if (isValidMove(newX, newY, 40, 40, &tank)) {
        tank.x = newX;
        tank.y = newY;
    } else {
        tank.direction = getRandomDirection();
    }
    tank.frame = (tank.frame + 1) % 2;
}

void updateAITanks(SDL_Renderer* renderer) {
    for (auto& tank : enemyTanks) {
        if (!tank.active) continue;

        if (tank.moveDelayCounter > 0) {
            tank.moveDelayCounter--;
        } else {
            tank.moveDelayCounter = 100;
            tank.direction = getRandomDirection();
        }
        moveAITank(tank);

        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - tank.lastShotTime > 2000) {
            tank.lastShotTime = currentTime;
            createBullet(tank, renderer);
        }
    }
}
void resetPlayerPosition() {
    playerTank.x = (float)40;
    playerTank.y = (float)40;
}
void updatePlayerTank() {
    if (playerTank.direction == NONE) return;

    float newX = playerTank.x;
    float newY = playerTank.y;

    switch (playerTank.direction) {
        case UP:    newY -= 2.0f; break;
        case DOWN:  newY += 2.0f; break;
        case LEFT:  newX -= 2.0f; break;
        case RIGHT: newX += 2.0f; break;
        case NONE:  break;
    }

    if (isValidMove(newX, newY, 40, 40, &playerTank)) {
        playerTank.x = newX;
        playerTank.y = newY;
    } else {
        playerTank.direction = NONE;
    }
    playerTank.frame = (playerTank.frame + 1) % 2;
}
