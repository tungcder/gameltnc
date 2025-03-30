#ifndef MENU_H
#define MENU_H

#include <SDL.h>
#include <vector>
#include "structs.h"

// Forward declaration
struct Button;

extern std::vector<Button> buttons;
void renderMenu(SDL_Renderer* renderer);
void handleMenuInput(const SDL_Event& e, GameState* gameState, SDL_Renderer* renderer);

#endif
