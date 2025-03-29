#ifndef INPUT_H
#define INPUT_H

#include <SDL.h>
#include "enum.h"
extern GameState gameState;

void handleMenuInput(const SDL_Event& e);
void handleGameInput(const SDL_Event& e);

#endif
