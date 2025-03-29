#include "ai.h"
#include "game.h"

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
