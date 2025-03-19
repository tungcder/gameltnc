#include <iostream>
#include <SDL.h>
#include<vector>


using namespace std;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int TILE_SIZE = 40;
const int MAP_WIDTH = SCREEN_WIDTH / TILE_SIZE;
const int MAP_HEIGHT = SCREEN_HEIGHT / TILE_SIZE; // Sửa lỗi đánh máy MAP_HEGHT -> MAP_HEIGHT


class Wall {
public:
    int x, y;
    SDL_Rect rect;
    bool active;
    int hp; // Máu của tường

    Wall(int startX, int startY) {
        x = startX;
        y = startY;
        active = true;
        hp = 2; // Tường cần 2 viên đạn mới vỡ
        rect = {x, y, TILE_SIZE, TILE_SIZE};
    }

    void takeDamage() {
        hp--;  // Giảm 1 máu khi trúng đạn
        if (hp <= 0) {
            active = false; // Nếu hết máu thì tường vỡ
        }
    }

    void render(SDL_Renderer* renderer) {
        if (active) {
            if (hp == 2) {
                SDL_SetRenderDrawColor(renderer, 150, 75, 0, 255); // Màu nâu đậm
            } else {
                SDL_SetRenderDrawColor(renderer, 200, 100, 50, 255); // Màu nâu nhạt (hư hỏng)
            }
            SDL_RenderFillRect(renderer, &rect);
        }
    }
};

class Bullet {
public:
    int x, y;
    int dirX, dirY;
    bool active;
    SDL_Rect rect;

    Bullet(int startX, int startY, int dX, int dY) {
        x = startX + TILE_SIZE / 2 - 5;  // Để viên đạn ở giữa xe tăng
        y = startY + TILE_SIZE / 2 - 5;
        dirX = dX;
        dirY = dY;
        active = true;
        rect = {x, y, 10, 10}; // Kích thước viên đạn
    }

    void update(vector<Wall>& walls) {
    if (active) {
        x += dirX * 5;
        y += dirY * 5;
        rect.x = x;
        rect.y = y;

        // Kiểm tra va chạm với tường
        for (Wall& wall : walls) {
    if (wall.active && SDL_HasIntersection(&rect, &wall.rect)) {
        wall.takeDamage(); // Gọi hàm giảm máu tường
        active = false; // Đạn biến mất sau khi bắn
        return;
    }
}


        if (x < 0 || x > SCREEN_WIDTH || y < 0 || y > SCREEN_HEIGHT) {
            active = false;
        }
    }
}

    void render(SDL_Renderer* renderer) {
        if (active) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_RenderFillRect(renderer, &rect);
        }
    }
};
class PlayerTank {
public:
    int x, y;
    int dirX, dirY;
    SDL_Rect rect;
    PlayerTank() {
        x = 0;
        y = 0;
        rect = {x, y, TILE_SIZE, TILE_SIZE};
        dirX = 0;
        dirY = -1;
    }

    PlayerTank (int startX, int startY) {
        x = startX;
        y = startY;
        rect = {x, y, TILE_SIZE, TILE_SIZE};
        dirX = 0;
        dirY = -1;
    }
    void move(int dx, int dy, const vector<Wall>& walls) {
        int newX = x + dx;
        int newY = y + dy;
        this->dirX = dx;
        this->dirY = dy;

        SDL_Rect newRect = {newX, newY, TILE_SIZE, TILE_SIZE};
        for (int i = 0; i < walls.size(); i++) {
            if(walls[i].active && SDL_HasIntersection(&newRect, &walls[i].rect)) {
                return;
            }
        }
        if (newX >= TILE_SIZE && newX <= SCREEN_WIDTH - TILE_SIZE * 2 &&
            newY >= TILE_SIZE && newY <= SCREEN_HEIGHT - TILE_SIZE * 2) {
                x = newX;
                y = newY;
                rect.x = x;
                rect.y = y;
            }
    }
    void shoot(vector<Bullet>& bullets) {
    if (dirX != 0 || dirY != 0) { // Kiểm tra xem xe tăng có hướng nào không
        bullets.push_back(Bullet(x, y, dirX, dirY));
    }
}


    void render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    SDL_RenderFillRect(renderer, &rect);
    }
};

class Game {
public:
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool running;
    vector<Wall> walls;
    PlayerTank player;
    vector<Bullet> bullets;


    void generateWalls() {
        for (int i=3; i < MAP_HEIGHT - 3; i+=2) {
            for (int j = 3; j < MAP_WIDTH - 3; j+=2) {
                Wall w = Wall(j * TILE_SIZE, i * TILE_SIZE);
                walls.push_back(w);
            }
        }
    }

    void handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
        } else if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case SDLK_UP: player.move(0, -10, walls); break;
                case SDLK_DOWN: player.move(0, 10, walls); break;
                case SDLK_LEFT: player.move(-10, 0, walls); break;
                case SDLK_RIGHT: player.move(10, 0, walls); break;
                case SDLK_SPACE: player.shoot(bullets); break;

            }
        }
    }
}
    Game() {
    running = true;
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << endl;
        running = false;
    }
    window = SDL_CreateWindow("Battle City", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH,
                              SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << endl;
        running = false;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << endl;
        running = false;
    }
    generateWalls();
    player = PlayerTank(((MAP_WIDTH - 1) / 2) * TILE_SIZE, (MAP_HEIGHT - 2) * TILE_SIZE);
}

// Render function
void render() {
    SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    for (int i = 1; i < MAP_HEIGHT - 1; ++i) {
        for (int j = 1; j < MAP_WIDTH - 1; ++j) {
            SDL_Rect tile = { j * TILE_SIZE, i * TILE_SIZE, TILE_SIZE, TILE_SIZE };
            SDL_RenderFillRect(renderer, &tile); // Vẽ ô vuông lên màn hình
        }
    }

    for (int i = 0; i < walls.size(); i++) {
        walls[i].render(renderer);
    }
    for (int i = 0; i < bullets.size(); i++) {
    bullets[i].render(renderer);
}

    player.render(renderer);
    SDL_RenderPresent(renderer);
}

// Run loop
void run() {
    const int FRAME_DELAY = 1000 / 60; // 60 FPS
    Uint32 frameStart;
    int frameTime;

    while (running) {
        frameStart = SDL_GetTicks(); // Lưu thời điểm bắt đầu frame

        handleEvents();

        // Cập nhật vị trí đạn
        for (int i = 0; i < bullets.size(); i++) {
            if (bullets[i].active) {
                bullets[i].update(walls);
            }
        }

        render();

        frameTime = SDL_GetTicks() - frameStart; // Tính thời gian frame chạy
        if (frameTime < FRAME_DELAY) {
            SDL_Delay(FRAME_DELAY - frameTime); // Chờ đến khi đủ thời gian cho 60 FPS
        }
    }
}


~Game() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
};



int main(int argc, char* argv[]) {
    Game game;
    if (game.running) {
        game.run();
    }
    return 0;
}
