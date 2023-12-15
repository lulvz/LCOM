#ifndef MENU_H
#define MENU_H

#include <stdint.h>

#include "game.h"
#include "gamestate.h"

uint16_t (*get_button_positions_menu())[4];

gamestate_t handle_click_main_menu(uint16_t x, uint16_t y);

#endif
