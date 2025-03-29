#include <SDL.h>
#include <iomanip>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <ctime>
#include <algorithm>
#include <cstdlib>
#include <sstream>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int TILE_SIZE = 40;
const int MAP_ROWS = SCREEN_HEIGHT / TILE_SIZE;
const int MAP_COLS = SCREEN_WIDTH / TILE_SIZE;
const int TANK_SIZE = 40;
const float TANK_SPEED = 2.0f;
const int FRAME_DELAY = 16;
const int BULLET_WIDTH = 10;
const int BULLET_HEIGHT = 10;
const float BULLET_SPEED = 6.0f;
const int EXPLOSION_WIDTH = 20;
const int EXPLOSION_HEIGHT = 20;
const int EXPLOSION_FRAMES = 3;
const int BULLET_COOLDOWN = 200;
const int AI_MOVE_DELAY = 100;
const int AI_SHOOT_COOLDOWN = 2000;
const int EXPLOSION_DELAY = 5;
const int PLAYER_SPRITE_WIDTH = 40;
const int PLAYER_SPRITE_HEIGHT = 40;
const int ENEMY_SPRITE_WIDTH = 40;
const int ENEMY_SPRITE_HEIGHT = 40;
const int PLAYER_LIVES = 2; // Số mạng của người chơi

enum TileType { EMPTY, BRICK, STEEL, WATER, TREE };
enum GameState { MENU, GAME, WIN, LOSE };
enum Direction { DOWN, LEFT, RIGHT, UP, NONE };

struct Button {
    SDL_Rect rect;
    SDL_Texture* texture;
};

struct Tank {
    float x, y;
    Direction direction;
    int frame;
    SDL_Texture* texture;
    Direction lastValidDirection;
    float targetX, targetY;
    bool isPlayer;
    Uint32 lastShotTime;
    int moveDelayCounter;
    bool active;
};

struct Bullet {
    float x, y;
    Direction direction;
    SDL_Texture* texture;
    bool active;
    bool isPlayerBullet;
};

struct Explosion {
    float x, y;
    int frame;
    int frameTimer;
    bool active;
};

SDL_Window* gWindow = nullptr;
SDL_Renderer* gRenderer = nullptr;
std::unordered_map<std::string, SDL_Texture*> gTextureCache;
TTF_Font* gFont = nullptr;  // Font để hiển thị số mạng
SDL_Color gTextColor = { 0, 0, 0 }; // Màu chữ trắng

int map[MAP_ROWS][MAP_COLS] = {
    {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {2,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,2},
    {2,0,3,3,0,0,4,4,0,0,0,0,0,4,4,0,3,3,0,2},
    {2,0,3,3,0,1,1,4,0,2,2,2,0,4,1,1,0,3,3,2},
    {2,0,0,0,0,1,0,0,0,2,2,2,0,0,0,1,0,0,0,2},
    {2,1,1,1,0,1,4,4,0,0,0,0,0,4,4,1,0,1,1,2},
    {2,0,0,0,0,1,4,4,0,1,1,1,0,4,4,1,0,0,0,2},
    {2,0,3,3,0,0,0,0,0,1,0,1,0,0,0,0,0,3,3,2},
    {2,0,3,3,0,1,1,1,0,1,0,1,0,1,1,1,0,3,3,2},
    {2,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,2},
    {2,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,2},
    {2,0,0,0,0,1,4,4,4,4,4,4,4,4,4,1,0,0,0,2},
    {2,0,3,3,0,0,0,0,0,1,0,1,0,0,0,0,0,3,3,2},
    {2,0,3,3,0,1,1,1,0,1,0,1,0,1,1,1,0,3,3,2},
    {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
};

std::vector<Explosion> explosions;
std::vector<Tank> enemyTanks;

GameState gameState = MENU;

std::vector<Button> buttons = {
    {{300, 340, 200, 50}, nullptr},
    {{300, 410, 200, 50}, nullptr},
    {{300, 480, 200, 50}, nullptr}
};

Tank playerTank = {
    (float)TILE_SIZE, (float)TILE_SIZE,
    NONE,
    0,
    nullptr,
    DOWN,
    (float)TILE_SIZE, (float)TILE_SIZE,
    true,
    0,
    0,
    true
};

int playerLives = PLAYER_LIVES; // Số mạng ban đầu của người chơi

std::vector<Bullet> bullets;
Uint32 lastBulletTime = 0;

Mix_Music* gMusic = nullptr;
Mix_Chunk* gBulletHitSound = nullptr;
Mix_Chunk* gGameOverSound = nullptr;

bool init();
SDL_Texture* loadTexture(const std::string& path);
SDL_Texture* getTexture(const std::string& path);
void close();
void renderMap();
void renderMenu();
void renderWin();
void renderLose();
void handleMenuInput(const SDL_Event& e);
void handleGameInput(const SDL_Event& e);
void updateGame();
void updatePlayerTank();
void renderGame();
void createBullet(Tank& tank);
void updateBullets();
void renderBullets();
bool isValidMove(float x, float y, int width, int height, Tank* currentTank = nullptr);
bool isTileBlocked(int row, int col);
void createExplosion(float x, float y);
void updateExplosions();
void renderExplosions();
Direction getRandomDirection();
void updateAITanks();
void moveAITank(Tank& tank);
bool checkCollision(float x1, float y1, int w1, int h1, float x2, float y2, int w2, int h2);
void resetPlayerPosition(); // Hàm đặt lại vị trí người chơi
void renderLives(SDL_Renderer* renderer, int lives); // Hàm hiển thị số mạng

bool loadMedia();

SDL_Texture* getTexture(const std::string& path) {
    if (gTextureCache.find(path) != gTextureCache.end()) {
        return gTextureCache[path];
    } else {
        SDL_Texture* texture = loadTexture(path);
        gTextureCache[path] = texture;
        return texture;
    }
}

bool init() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cout << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }
    gWindow = SDL_CreateWindow("Battle City", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!gWindow) {
        std::cout << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }
    gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
    if (!gRenderer) {
        std::cout << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cout << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
        return false;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cout << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return false;
    }

    // Initialize SDL_ttf
    if (TTF_Init() == -1) {
        std::cout << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return false;
    }

    gFont = TTF_OpenFont("font/Hack.ttf", 24);  // Thay "font/Hack.ttf" bằng đường dẫn đến font của bạn
    if (gFont == nullptr) {
        std::cout << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return false;
    }
    srand(time(NULL));
    return true;
}

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

bool loadMedia() {
    gMusic = Mix_LoadMUS("sound/intro.mp3");
    if (!gMusic) {
        std::cout << "Failed to load intro music! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return false;
    }
    gBulletHitSound = Mix_LoadWAV("sound/noo.wav");
    if (!gBulletHitSound) {
        std::cout << "Failed to load bullet hit sound effect! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return false;
    }
    gGameOverSound = Mix_LoadWAV("sound/gameover.wav");
    if (!gGameOverSound) {
        std::cout << "Failed to load game over sound effect! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return false;
    }
    return true;
}

void close() {
    for (auto const& [key, val] : gTextureCache) {
        SDL_DestroyTexture(val);
    }
    gTextureCache.clear();
    Mix_FreeChunk(gBulletHitSound);
    Mix_FreeChunk(gGameOverSound);
    gBulletHitSound = nullptr;
    gGameOverSound = nullptr;
    Mix_FreeMusic(gMusic);
    gMusic = nullptr;
    TTF_CloseFont(gFont); // Giải phóng font
    TTF_Quit();
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    Mix_Quit();
    IMG_Quit();
    SDL_Quit();
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
                    resetPlayerPosition(); // Đặt lại vị trí xe tăng người chơi khi bắt đầu game
                    playerLives = PLAYER_LIVES;
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

bool isValidMove(float x, float y, int width, int height, Tank* currentTank) {
    int topLeftX = (int)floor(x / TILE_SIZE);
    int topLeftY = (int)floor(y / TILE_SIZE);
    int topRightX = (int)floor((x + width - 1) / TILE_SIZE);
    int topRightY = (int)floor(y / TILE_SIZE);
    int bottomLeftX = (int)floor(x / TILE_SIZE);
    int bottomLeftY = (int)floor((y + height - 1) / TILE_SIZE);
    int bottomRightX = (int)floor((x + width - 1) / TILE_SIZE);
    int bottomRightY = (int)floor((y + height - 1) / TILE_SIZE);

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

            if (checkCollision(x, y, width, height, enemyTank.x, enemyTank.y, TANK_SIZE, TANK_SIZE)) {
                return false;
            }
        }
    } else {
        if (playerTank.active && checkCollision(x, y, width, height, playerTank.x, playerTank.y, TANK_SIZE, TANK_SIZE)) {
            return false;
        }
        for (auto& enemyTank : enemyTanks) {
            if (!enemyTank.active) continue;
            if (&enemyTank == currentTank) continue; // Bỏ qua kiểm tra với chính nó

            if (checkCollision(x, y, width, height, enemyTank.x, enemyTank.y, TANK_SIZE, TANK_SIZE)) {
                return false;
            }
        }
    }
    return true;
}

bool isTileBlocked(int row, int col) {
    if (row < 0 || row >= MAP_ROWS || col < 0 || col >= MAP_COLS) {
        return true;
    }
    return map[row][col] != EMPTY && map[row][col] != TREE;
}
bool checkCollision(float x1, float y1, int w1, int h1, float x2, float y2, int w2, int h2) {
    return (x1 < x2 + w2 &&
            x1 + w1 > x2 &&
            y1 < y2 + h2 &&
            y1 + h1 > y2);
}

void resetPlayerPosition() {
    playerTank.x = (float)TILE_SIZE;
    playerTank.y = (float)TILE_SIZE;
}

void updateGame() {
    if (gameState == GAME) {
        updatePlayerTank();
        updateAITanks();
        updateBullets();
        if (enemyTanks.empty()) {
            gameState = WIN;
            Mix_HaltMusic();
            Mix_PlayChannel(-1, gGameOverSound, 0);
        }
        if (playerLives <= 0) {
            gameState = LOSE;
            Mix_HaltMusic();
            Mix_PlayChannel(-1, gGameOverSound, 0);
        }
    }
}

void updatePlayerTank() {
    if (playerTank.direction == NONE) return;

    float newX = playerTank.x;
    float newY = playerTank.y;

    switch (playerTank.direction) {
        case UP:    newY -= TANK_SPEED; break;
        case DOWN:  newY += TANK_SPEED; break;
        case LEFT:  newX -= TANK_SPEED; break;
        case RIGHT: newX += TANK_SPEED; break;
        case NONE:  break;
    }

    if (isValidMove(newX, newY, TANK_SIZE, TANK_SIZE, &playerTank)) {
        playerTank.x = newX;
        playerTank.y = newY;
    } else {
        playerTank.direction = NONE;
    }
    playerTank.frame = (playerTank.frame + 1) % 2;
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

            if (explosion.frameTimer >= EXPLOSION_DELAY) {
                explosion.frameTimer = 0;
                explosion.frame++;

                if (explosion.frame >= EXPLOSION_FRAMES) {
                    explosion.active = false;
                }
            }
        }
    }
    explosions.erase(std::remove_if(explosions.begin(), explosions.end(), [](const Explosion& e) { return !e.active; }), explosions.end());
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

void createBullet(Tank& tank) {
    Bullet newBullet;
    newBullet.x = tank.x + TANK_SIZE / 2 - BULLET_WIDTH / 2;
    newBullet.y = tank.y + TANK_SIZE / 2 - BULLET_HEIGHT / 2;
    newBullet.direction = (tank.direction == NONE) ? tank.lastValidDirection : tank.direction;
    newBullet.texture = getTexture("image/bullet.png");
    newBullet.active = true;
    newBullet.isPlayerBullet = tank.isPlayer;

    bullets.push_back(newBullet);
}

void updateBullets() {
    for (auto it = bullets.begin(); it != bullets.end();) {
        Bullet& bullet = *it;
        if (!bullet.active) {
            it = bullets.erase(it);
            continue;
        }
        switch (bullet.direction) {
            case UP: bullet.y -= BULLET_SPEED; break;
            case DOWN: bullet.y += BULLET_SPEED; break;
            case LEFT: bullet.x -= BULLET_SPEED; break;
            case RIGHT: bullet.x += BULLET_SPEED; break;
            case NONE: bullet.active = false; break;
        }
        if (bullet.x < 0 || bullet.x > SCREEN_WIDTH || bullet.y < 0 || bullet.y > SCREEN_HEIGHT) {
            bullet.active = false;
        }
        int tileX = (int)(bullet.x / TILE_SIZE);
        int tileY = (int)(bullet.y / TILE_SIZE);

        if (tileX >= 0 && tileX < MAP_COLS && tileY >= 0 && tileY < MAP_ROWS) {
            if (map[tileY][tileX] == BRICK) {
                map[tileY][tileX] = EMPTY;
                bullet.active = false;
                createExplosion(tileX * TILE_SIZE, tileY * TILE_SIZE);
                Mix_PlayChannel(-1, gBulletHitSound, 0);

            } else if (map[tileY][tileX] != EMPTY && map[tileY][tileX] != TREE) {
                bullet.active = false;
                createExplosion(bullet.x - BULLET_WIDTH / 2, bullet.y - BULLET_HEIGHT / 2);
                Mix_PlayChannel(-1, gBulletHitSound, 0);
            }
        }
        if (bullet.isPlayerBullet) {
            for (auto& enemyTank : enemyTanks) {
                if (!enemyTank.active) continue;
                if (checkCollision(bullet.x, bullet.y, BULLET_WIDTH, BULLET_HEIGHT, enemyTank.x, enemyTank.y, TANK_SIZE, TANK_SIZE)) {
                    bullet.active = false;
                    createExplosion(enemyTank.x, enemyTank.y);
                    enemyTank.active = false;
                    Mix_PlayChannel(-1, gBulletHitSound, 0);
                    break;
                }
            }
        } else {
            if (playerTank.active && checkCollision(bullet.x, bullet.y, BULLET_WIDTH, BULLET_HEIGHT, playerTank.x, playerTank.y, TANK_SIZE, TANK_SIZE)) {
                bullet.active = false;
                createExplosion(playerTank.x, playerTank.y);
                playerTank.active = false;
                playerLives--; // Giảm số mạng khi xe tăng người chơi bị bắn
                if (playerLives > 0) {
                    resetPlayerPosition(); // Đặt lại vị trí nếu còn mạng
                    playerTank.active = true;
                }
                Mix_PlayChannel(-1, gBulletHitSound, 0);
            }
        }
         ++it;
    }
    bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [](const Bullet& b) { return !b.active; }), bullets.end());
    enemyTanks.erase(std::remove_if(enemyTanks.begin(), enemyTanks.end(), [](const Tank& t) { return !t.active; }), enemyTanks.end());
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
        case UP:    newY -= TANK_SPEED; break;
        case DOWN:  newY += TANK_SPEED; break;
        case LEFT:  newX -= TANK_SPEED; break;
        case RIGHT: newX += TANK_SPEED; break;
        case NONE:  break;
    }

    if (isValidMove(newX, newY, TANK_SIZE, TANK_SIZE, &tank)) {
        tank.x = newX;
        tank.y = newY;
    } else {
        tank.direction = getRandomDirection();
    }
    tank.frame = (tank.frame + 1) % 2;
}

void updateAITanks() {
    for (auto& tank : enemyTanks) {
        if (!tank.active) continue;

        if (tank.moveDelayCounter > 0) {
            tank.moveDelayCounter--;
        } else {
            tank.moveDelayCounter = AI_MOVE_DELAY;
            tank.direction = getRandomDirection();
        }
        moveAITank(tank);

        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - tank.lastShotTime > AI_SHOOT_COOLDOWN) {
            tank.lastShotTime = currentTime;
            createBullet(tank);
        }
    }
}

void renderLives(SDL_Renderer* renderer, int lives) {
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
    updateExplosions();
    renderExplosions();
    renderLives(gRenderer, playerLives); // Hiển thị số mạng của người chơi
}

int main(int argc, char* args[]) {
    if (!init()) {
        std::cout << "Failed to initialize!" << std::endl;
        return -1;
    }

    if (!loadMedia()) {
        std::cout << "Failed to load media!" << std::endl;
        close();
        return -1;
    }
    Mix_PlayMusic(gMusic, -1);

    buttons[0].texture = getTexture("image/1player.png");
    buttons[1].texture = getTexture("image/2player.png");
    buttons[2].texture = getTexture("image/exit.png");

    std::string playerPath = "image/Tank.png";
    gTextureCache[playerPath] = loadTexture(playerPath.c_str());
    playerTank.texture = gTextureCache[playerPath];
    if (!playerTank.texture) {
        std::cout << "Failed to load player tank sprite sheet!" << std::endl;
        close();
        return -1;
    }
    SDL_Texture* bulletTexture = getTexture("image/bullet.png");
    if (!bulletTexture) {
        std::cout << "Failed to load bullet sprite sheet!" << std::endl;
        close();
        return -1;
    }
        SDL_Texture* explosionTexture = getTexture("image/no.png");
    if (!explosionTexture) {
        std::cout << "Failed to load explosion sprite sheet!" << std::endl;
        close();
        return -1;
    }

    SDL_Texture* winTexture = getTexture("image/win.png");
    if (!winTexture) {
        std::cout << "Failed to load win image!" << std::endl;
        close();
        return -1;
    }

    SDL_Texture* loseTexture = getTexture("image/lose.png");
    if (!loseTexture) {
        std::cout << "Failed to load lose image!" << std::endl;
        close();
        return -1;
    }
    bool quit = false;
    SDL_Event e;

    while (!quit) {
        Uint32 frameStart = SDL_GetTicks();
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }

            switch (gameState) {
                case MENU:
                    handleMenuInput(e);
                    break;
                case GAME:
                    handleGameInput(e);
                    break;
                case WIN:
                    if (e.type == SDL_MOUSEBUTTONDOWN) {
                       gameState = MENU;
                       Mix_HaltMusic();
                       Mix_PlayMusic(gMusic, -1);
                    }
                    break;
                case LOSE:
                    if (e.type == SDL_MOUSEBUTTONDOWN) {
                        gameState = MENU;
                       Mix_HaltMusic();
                       Mix_PlayMusic(gMusic, -1);
                    }
                    break;
                default:
                    break;
            }
        }

        if (gameState == GAME) {
            updateGame();
        }

        SDL_RenderClear(gRenderer);

        switch (gameState) {
            case MENU:
                renderMenu();
                break;
            case GAME:
                renderGame();
                break;
            case WIN:
                renderWin();
                break;
            case LOSE:
                renderLose();
                break;
            default:
                break;
        }

        SDL_RenderPresent(gRenderer);
        Uint32 frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < FRAME_DELAY) {
            SDL_Delay(FRAME_DELAY - frameTime);
        }
    }
    close();
    return 0;
}
