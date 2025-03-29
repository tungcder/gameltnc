#include "graphics.h"
#include "audio.h"
#include "game.h"
#include <SDL_mixer.h>

extern SDL_Renderer* gRenderer;
extern std::unordered_map<std::string, SDL_Texture*> gTextureCache;
extern GameState gameState;
extern std::vector<Button> buttons;
extern Tank playerTank;
extern std::vector<Tank> enemyTanks;
extern std::vector<Bullet> bullets;
extern std::vector<Explosion> explosions;
SDL_Texture* loadTexture(const std::string& path) {
    SDL_Texture* newTexture = nullptr;
    SDL_Surface* loadedSurface = IMG_Load(path.c_str());
    if (!loadedSurface) {
        std::cout << "Unable to load image " << path << "! SDL_image Error: " << IMG_GetError() << std::endl;
        return nullptr;
    } else {
        newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
        SDL_FreeSurface(loadedSurface);
    }
    return newTexture;
}
SDL_Texture* getTexture(const std::string& path) {
    if (gTextureCache.find(path) != gTextureCache.end()) {
        return gTextureCache[path];
    } else {
        SDL_Texture* texture = loadTexture(path);
        gTextureCache[path] = texture;
        return texture;
    }
}
void renderMap() {
    for (int row = 0; row < MAP_ROWS; ++row) {
        for (int col = 0; col < MAP_COLS; ++col) {
            SDL_Rect tileRect = {col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE};
            SDL_Texture* texture = nullptr;

            switch (map[row][col]) {
                case BRICK: texture = getTexture("image/wall1.png"); break;
                case STEEL: texture = getTexture("image/wall2.png"); break;
                case WATER: texture = getTexture("image/water.png"); break;
                case TREE:  texture = getTexture("image/tree.png"); break;
                default: break;
            }

            if (texture) {
                SDL_RenderCopy(gRenderer, texture, nullptr, &tileRect);
            }
        }
    }
}

void renderMenu() {
    SDL_RenderCopy(gRenderer, getTexture("image/background.png"), nullptr, nullptr);
    for (const auto& button : buttons) {
        SDL_RenderCopy(gRenderer, button.texture, nullptr, &button.rect);
    }
}

void renderWin() {
    SDL_RenderCopy(gRenderer, getTexture("image/win.png"), nullptr, nullptr);
}

void renderLose() {
    SDL_RenderCopy(gRenderer, getTexture("image/lose.png"), nullptr, nullptr);
}

void renderBullets() {
    SDL_Texture* bulletTexture = getTexture("image/bullet.png");

    for (const auto& bullet : bullets) {
        if (bullet.active && bulletTexture) {
            SDL_Rect bulletRect;
            bulletRect.w = BULLET_WIDTH;
            bulletRect.h = BULLET_HEIGHT;
            bulletRect.x = (int)bullet.x;
            bulletRect.y = (int)bullet.y;

            SDL_Rect bulletSourceRect;
            bulletSourceRect.w = 10;
            bulletSourceRect.h = 10;
            switch (bullet.direction) {
                case DOWN:
                    bulletSourceRect.x = 0;
                    bulletSourceRect.y = 0;
                    break;
                case LEFT:
                    bulletSourceRect.x = 10;
                    bulletSourceRect.y = 0;
                    break;
                case RIGHT:
                    bulletSourceRect.x = 20;
                    bulletSourceRect.y = 0;
                    break;
                case UP:
                    bulletSourceRect.x = 30;
                    bulletSourceRect.y = 0;
                    break;
                case NONE:
                   bulletSourceRect.x = 0;
                    bulletSourceRect.y = 0;
                    break;
            }
            SDL_RenderCopy(gRenderer, bulletTexture, &bulletSourceRect, &bulletRect);
        }
    }
}

void renderExplosions() {
    SDL_Texture* explosionTexture = getTexture("image/no.png");
    if (!explosionTexture) return;

    for (const auto& explosion : explosions) {
        if (explosion.active) {
            SDL_Rect srcRect;
            srcRect.x = explosion.frame * EXPLOSION_WIDTH;
            srcRect.y = 0;
            srcRect.w = EXPLOSION_WIDTH;
            srcRect.h = EXPLOSION_HEIGHT;

            SDL_Rect destRect;
            destRect.x = (int)explosion.x;
            destRect.y = (int)explosion.y;
            destRect.w = EXPLOSION_WIDTH;
            destRect.h = EXPLOSION_HEIGHT;

            SDL_RenderCopy(gRenderer, explosionTexture, &srcRect, &destRect);
        }
    }
}

void renderGame() {
    Direction renderDirection = (playerTank.direction == NONE) ? playerTank.lastValidDirection : playerTank.direction;

    int yOffset = 0;
    switch (renderDirection) {
        case DOWN:  yOffset = 0; break;
        case LEFT:  yOffset = 1; break;
        case RIGHT: yOffset = 2; break;
        case UP:    yOffset = 3; break;
        case NONE: break;
    }

    SDL_Rect spriteRect;
    spriteRect.w = TANK_SIZE;
    spriteRect.h = TANK_SIZE;
    spriteRect.x = playerTank.frame * TANK_SIZE;
    spriteRect.y = (yOffset * TILE_SIZE);

    renderMap();

    SDL_Rect renderRect;
    renderRect.x = (int)playerTank.x;
    renderRect.y = (int)playerTank.y;
    renderRect.w = TANK_SIZE;
    renderRect.h = TANK_SIZE;

    if (playerTank.active) {
         SDL_RenderCopy(gRenderer, playerTank.texture, &spriteRect, &renderRect);
    }
    for (const auto& tank : enemyTanks) {
        if (tank.active) {
            Direction renderEnemyDirection = (tank.direction == NONE) ? tank.lastValidDirection : tank.direction;

            int yOffsetEnemy = 0;
            switch (renderEnemyDirection) {
                case DOWN:  yOffsetEnemy = 0; break;
                case LEFT:  yOffsetEnemy = 1; break;
                case RIGHT: yOffsetEnemy = 2; break;
                case UP:    yOffsetEnemy = 3; break;
                case NONE: break;
            }

            SDL_Rect spriteRectEnemy;
            spriteRectEnemy.w = ENEMY_SPRITE_WIDTH;
            spriteRectEnemy.h = ENEMY_SPRITE_HEIGHT;
            spriteRectEnemy.x = tank.frame * ENEMY_SPRITE_WIDTH;
            spriteRectEnemy.y = (yOffsetEnemy * TILE_SIZE);

            SDL_Rect renderRectEnemy;
            renderRectEnemy.x = (int)tank.x;
            renderRectEnemy.y = (int)tank.y;
            renderRectEnemy.w = TANK_SIZE;
            renderRectEnemy.h = TANK_SIZE;

            SDL_RenderCopy(gRenderer, tank.texture, &spriteRectEnemy, &renderRectEnemy);
        }
    }
    renderBullets();
    renderExplosions();
}
