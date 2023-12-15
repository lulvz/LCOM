#include "menu.h"

extern gamestate_t gamestate;

/// @brief Array with the positions of the buttons in the main menu.
static uint16_t buttons[3][4] = {
  { 500, 250, 150, 80 }, // START BUTTON
  { 500, 350, 150, 80 }, // MULTI BUTTON
  { 500, 450, 150, 80 } // EXIT BUTTON
};

/// @brief Gets the positions of the buttons in the main menu.
/// @return Returns the positions of the buttons in the main menu.
uint16_t (*get_button_positions_menu())[4] {
  return buttons;
}

/// @brief Handles a click in the main menu, swithing to the appropriate gamestate.
/// @param x X coordinate of the click.
/// @param y Y coordinate of the click.
/// @return Returns the gamestate to which the game should switch.
gamestate_t handle_click_main_menu(uint16_t x, uint16_t y) {
  for (int i = 0; i < 3; i++) {
    if ((x >= buttons[i][0]) && (x <= buttons[i][0] + buttons[i][2]) &&
        (y >= buttons[i][1]) && (y <= buttons[i][1] + buttons[i][3])) {
      switch (i) {
        case 0: {
          load_random_race();
          return SINGLE;
        }
        case 1: {
          load_random_race();
          return MULTI;
        }
        case 2: {
          return EXIT;
        }
      }
    }
  }

  return MAIN_MENU;
}
