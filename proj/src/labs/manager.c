#include "manager.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "i8042.h"
#include "keyboard.h"
#include "video_gr.h"
#include "rtc.h"
#include <lcom/timer.h>

#include "game/game.h"
#include "game/menu.h"
#include "game/gamestate.h"
#include "game/gameover.h"

#include <lcom/lcf.h>

#include "../game/img/bg.xpm"
#include "../game/img/c.xpm" // creates a static const char* cursor[] with an xpm formatted c string
#include "../game/img/dbg.xpm"

#include "../game/img/bubble.xpm"
#include "../game/img/bubble_small.xpm"
#include "../game/img/bubble_big.xpm"

//buttons xpm
#include "../game/img/START_button.xpm"
#include "../game/img/MULTI_button.xpm"
#include "../game/img/EXIT_button.xpm"
#include "../game/img/RETRY_button.xpm"
#include "../game/img/MENU_button.xpm"


/// @brief Structs used for storing the various sprites in the game.
struct sprite *bg, *crs, *dbg, *start_button, *multi_button, *exit_button, *retry_button, *menu_button;

struct animated_sprite *bubble = NULL;

bool exited = false;

int irq_set_timer = 0, irq_set_keyboard = 0, irq_set_mouse = 0, irq_set_rtc = 0;
extern gamestate_t gamestate;

/// @brief Simultaniously subscribes to all interrupts.
/// @return Returns 0 upon success and non-zero otherwise.
int subscribe_ints(void) {
  // temporary value to hold the number of the set bit
  uint8_t bit_no;
  if (timer_subscribe_int(&bit_no) != OK) {
    printf("Error in timer_subscribe_int\n");
    return 1;
  }
  irq_set_timer = BIT(bit_no);

  if (keyboard_subscribe_int(&bit_no) != OK) {
    printf("Error in keyboard_subscribe_int\n");
    return 1;
  }
  irq_set_keyboard = BIT(bit_no);

  if (mouse_subscribe_int(&bit_no) != OK) {
    printf("Error in mouse_subscribe_int\n");
    return 1;
  }
  irq_set_mouse = BIT(bit_no);

  if(rtc_subscribe_int(&bit_no) != OK){
    printf("Error in rtc_subscribe_int\n");
    return 1;
  }
  irq_set_rtc = BIT(bit_no);

  return OK;
}

/// @brief Simultaniously intializes all the sprites, video card, RTC (real-time clock) and calls subscribe_ints().
/// This function sets the framerate to 60 and enables data reporting for the mouse.
/// @return Returns 0 upon success and non-zero otherwise.
int init_all(void) {
  crs = create_sprite("crs", c_xpm, 222, 222, 0, 0);
  bg = create_sprite("bg", bg_xpm, 0, 0, 0, 0);
  dbg = create_sprite("dbg", dbg_xpm, 0, 0, 0, 0);

  // buttons
  // get positions of the buttons from the menus
  uint16_t (*button_positions_menu)[4] = get_button_positions_menu();
  uint16_t (*button_positions_game_over)[4] = get_button_positions_game_over();
  // check for null
  if ((button_positions_menu == NULL) || (button_positions_game_over == NULL)) {
    printf("Error in get_button_positions\n");
    return 1;
  }

  // print the positions of the buttons
  for (int i = 0; i < 3; i++) {
    printf("button %d: x: %d, y: %d, width: %d, height: %d\n", i, button_positions_menu[i][0], button_positions_menu[i][1], button_positions_menu[i][2], button_positions_menu[i][3]);
  }

  start_button = create_sprite("start_button", START_button_xpm, button_positions_menu[0][0], button_positions_menu[0][1], 0, 0);
  multi_button = create_sprite("multi_button", MULTI_button_xpm, button_positions_menu[1][0], button_positions_menu[1][1], 0, 0);
  exit_button = create_sprite("exit_button", EXIT_button_xpm, button_positions_menu[2][0], button_positions_menu[2][1], 0, 0);

  retry_button = create_sprite("retry_button", RETRY_button_xpm, button_positions_game_over[0][0], button_positions_game_over[0][1], 0, 0);
  menu_button = create_sprite("menu_button", MENU_button_xpm, button_positions_game_over[1][0], button_positions_game_over[1][1], 0, 0);

  timer_set_frequency(0, 60);

  mouse_enable_data_reporting_custom();

  if (subscribe_ints() != OK) {
    printf("Error in subscribe_ints\n");
    return 1;
  }

  // initialize rtc
  if(initialize_rtc() != OK){
    printf("Error in initialize_rtc\n");
    return 1;
  }

  if (vg_init(0x14C) == NULL) {
    printf("Error in vg_init\n");
    return 1;
  }

  return OK;
}

/// @brief Simultaniously unsubscribes to all interrupts.
/// @return Returns 0 upon success and non-zero otherwise.
int unsubscribe_ints(void) {
  if (timer_unsubscribe_int() != OK) {
    printf("Error in timer_unsubscribe_int\n");
    return 1;
  }

  if (keyboard_unsubscribe_int() != OK) {
    printf("Error in keyboard_unsubscribe_int\n");
    return 1;
  }

  if (mouse_unsubscribe_int() != OK) {
    printf("Error in mouse_unsubscribe_int\n");
    return 1;
  }

  if(rtc_unsubscribe_int() != OK){
    printf("Error in rtc_unsubscribe_int\n");
    return 1;
  }

  return OK;
}

/// @brief Simultanioulsy destroys all the sprites, returns to text mode and calls unsubscribe_ints().
/// @return Returns 0 upon success and non-zero otherwise.
int end_all(void) {
  if (unsubscribe_ints() != OK) {
    printf("Error in unsubscribe_ints\n");
    return 1;
  }

  if (vg_exit_custom() != OK) {
    printf("Error in vg_exit\n");
    return 1;
  }

  kbd_enable();

  destroy_sprite(bg);
  destroy_sprite(crs);
  destroy_sprite(dbg);
  destroy_sprite(start_button);
  destroy_sprite(multi_button);
  destroy_sprite(exit_button);
  destroy_sprite(retry_button);
  destroy_sprite(menu_button);

  // check for bubbles
  if (bubble != NULL) {
    destroy_animated_sprite(bubble);
  }

  return OK;
}

bool redraw = true;
extern char full_sentence[MAX_TOTAL_CHARS];

/// @brief Function that handles the main game loop.
/// This function handles the interrupts appropriately.
/// @return Returns 0 upon success and non-zero otherwise.
int main_game_loop(void) {
  int r, ipc_status;
  message msg;

  while (!exited) {
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != OK) {
      printf("Error in driver_receive: %d", r);
      return 1;
    }
    if (is_ipc_notify(ipc_status)) {
      if (_ENDPOINT_P(msg.m_source) == HARDWARE) {
        if (msg.m_notify.interrupts & irq_set_timer) {
          timer_int();
        }
        if (msg.m_notify.interrupts & irq_set_keyboard) {
          keyboard_int();
        }
        if (msg.m_notify.interrupts & irq_set_mouse) {
          mouse_int();
        }
        if(msg.m_notify.interrupts & irq_set_rtc){
          printf("rtc interrupt\n");
          rtc_ih();
        }
      }
    }
  }

  return OK;
}

extern long counter;
extern char rows[MAX_ROWS][MAX_ROW_CHARS + 1];
extern int current_word_idx;
extern char user_word[100];
extern char final_user_sentence[MAX_TOTAL_CHARS * 3];

/// @brief Draws a word in the screen.
void draw_user_word() {
  uint32_t color = 0x00000000;
  if (cmp_current_word() != OK) {
    // word is not the same, draw underline in red
    color = 0x00FF0000;
  }
  else {
    color = 0x0000FF00;
  }
  vg_draw_string(500, 700, user_word, color);
}

/// @brief Draws an underline under the character that the user is currently typing.
void draw_underline(int current_word_idx) {
  // draw an underline on the first char of the current word
  // use current_word_idx%MAX_ROW_CHARS to get the index of the char in the current row

  uint32_t color = 0x00000000;
  if (cmp_current_word() != OK) {
    // word is not the same, draw underline in red
    color = 0x00FF0000;
  }
  else {
    color = 0x0000FF00;
  }

  vg_draw_hline(
    50 + ((current_word_idx) % MAX_ROW_CHARS) * 16,
    (100 + ((current_word_idx) / MAX_ROW_CHARS) * 25) + 24,
    16,
    color);
  vg_draw_hline(
    50 + ((current_word_idx) % MAX_ROW_CHARS) * 16,
    (100 + ((current_word_idx) / MAX_ROW_CHARS) * 25) + 25,
    16,
    color);
  vg_draw_hline(
    50 + ((current_word_idx) % MAX_ROW_CHARS) * 16,
    (100 + ((current_word_idx) / MAX_ROW_CHARS) * 25) + 23,
    16,
    color);
}

/// @brief Timer interrupt handler.
/// This function defines how timer interrupts are handled, depending on the current gamestate.
void timer_int() {
  timer_int_handler(); // standard timer interrupt handler

  switch (gamestate) {
    case MAIN_MENU: {// main menu
      if(redraw) {
        vg_draw_sprite(dbg);

        vg_draw_sprite(start_button);
        vg_draw_sprite(multi_button);
        vg_draw_sprite(exit_button);

        // print current date/time on top of screen
        char date_time[17] = {0};
        get_time_until_minutes(date_time);
        vg_draw_string(50, 50, date_time, 0x00FFFFFF);

        vg_draw_sprite(crs);
        redraw = false;
      }

      copy_mem();
      break;
    }
    case SINGLE: { // only keyboard
      // increase game counter
      increase_game_counter();

      if(get_game_counter() % 60 == 0) {
        // a second has passed, so update the screen to show
        redraw = true;
      }

      if (redraw) {
        vg_draw_sprite(dbg);
        vg_draw_sprite(crs);

        int i = 0;
        while(rows[i][0] != '\0') {
          vg_draw_string(50, 100 + i * 25, rows[i], 0x00FFFFFF);
          i++;
        }

        draw_user_word();

        draw_underline(current_word_idx);

        vg_draw_string(390, 600, "TIME (s): ", 0x00FFFFFF);
        char tmp[10];
        sprintf(tmp, "%lu", get_game_counter()/60);
        vg_draw_string(570, 600, tmp, 0x008f3322);

        redraw = false;
      }

      copy_mem();

      break;
    }
    case MULTI: { // keyboard and mouse
      increase_game_counter();
      // temporary, to be replaced by a check for an alarm for bubble clearance
      if(counter % 180 == 0) {
        if(bubble != NULL) {
          destroy_animated_sprite(bubble);
        }
        bubble = NULL;
        redraw = true;
      }
      if(get_game_counter() % 60 == 0) {
        // a second has passed, so update the screen to show
        redraw = true;
      }

      vg_draw_sprite(dbg);
      vg_draw_sprite(crs);

      int i = 0;
      while(rows[i][0] != '\0') {
        vg_draw_string(50, 100 + i * 25, rows[i], 0x00FFFFFF);
        i++;
      }

      draw_user_word();

      draw_underline(current_word_idx);

      vg_draw_string(390, 600, "TIME (s): ", 0x00FFFFFF);
      char tmp[10];
      sprintf(tmp, "%lu", get_game_counter()/60);
      vg_draw_string(570, 600, tmp, 0x008f3322);

      if(bubble != NULL) {
        vg_draw_animated_sprite(bubble);
      }

      copy_mem();

      break;
    }
    case GAME_OVER: {
      if(!is_score_generated()) {
        gamestate = MAIN_MENU;
        break;
      }
      // draw game over screen
      if(redraw) {
        vg_draw_sprite(dbg);

        // get score and draw it
        unsigned int correct_words = get_final_correct_words();
        unsigned int total_words = get_total_words();
        unsigned int correct_chars = get_final_correct_chars();
        unsigned int total_chars = get_total_chars();
        float final_wpm = get_final_wpm();

        char tmp[100];
        sprintf(tmp, "Correct words: %d out of %d", correct_words, total_words);
        vg_draw_string(650, 100, tmp, 0x00FFFFFF);
        sprintf(tmp, "Correct chars: %d out of %d", correct_chars, total_chars);
        vg_draw_string(650, 150, tmp, 0x00FFFFFF);
        sprintf(tmp, "WPM: %.1f", final_wpm);
        vg_draw_string(650, 200, tmp, 0x00FFFFFF);

        vg_draw_string(650, 400, "Enter your name: ", 0x00FFFFFF);
        vg_draw_string(650, 450, get_user_name(), 0x00FFFFFF);

        // show top 10 highscores on left side of screen in a list
        highscore_t highscores[10];
        int num_highscores = get_top10_highscores(highscores);

        for(int i = 0; i<num_highscores; i++) {
          // format parts into a single string to draw
          char tmp[256];
          if(strlen(highscores[i].name) > 6) {
            highscores[i].name[6] = '\0';
          }
          sprintf(tmp, "%d. %s - %.1f - %s", i+1, highscores[i].name, highscores[i].wpm, highscores[i].date);
          vg_draw_string(50, 100 + i*50, tmp, 0x00FF77FF);
        }

        vg_draw_sprite(retry_button);
        vg_draw_sprite(menu_button);

        vg_draw_sprite(crs);
        redraw = false;
      }

      copy_mem();
      break;
    }
    case EXIT: {
      exited = true;
    }
    default:
      break;
  }
}

/// @brief Handles the keyboard interrupts.
/// Appropriately handles the keyboard interrupts depending on the gamestate.
void keyboard_int() {
  switch (gamestate) {
    case SINGLE: case MULTI: {
      // function that handles the keyboard interrupt
      kbc_ih();
      event_t current_event = kbc_get_event();

      game_handle_event(current_event);
      redraw = true;
      break;
    }
    case MAIN_MENU: { // do nothing in main menu
      kbc_ih();
      event_t current_event = kbc_get_event();

      if(current_event.action == EXIT_PROGRAM) {
        exited = true;
        return;
      }
      break;
    }
    case GAME_OVER: {
      kbc_ih();
      event_t current_event = kbc_get_event();

      game_over_handle_event(current_event);
      redraw = true;
      break;
    }
    default:
      break;
  }
}

extern struct packet p;

/// @brief Handles the mouse interrupts.
/// Appropriately handles the mouse interrupts depending on the gamestate.
void mouse_int() {
  // returns true when the packet is fully built
  if (mouse_stream_handler()) {
    // vg_erase_sprite(crs);
    // check if uint16_t in crs->x or crs->y is overflowed
    if (((int16_t) crs->x + (int16_t) p.delta_x) < 0) {
      p.delta_x = 0;
      crs->x = 0;
    }
    if (((int16_t) crs->x + (int16_t) p.delta_x) > ((int) get_h_res() - 1)) {
      p.delta_x = 0;
      crs->x = get_h_res() - 1;
    }
    if (((int16_t) crs->y - (int16_t) p.delta_y) < 0) {
      p.delta_y = 0;
      crs->y = 0;
    }
    if (((int16_t) crs->y - (int16_t) p.delta_y) > ((int) get_v_res() - 1)) {
      p.delta_y = 0;
      crs->y = get_v_res() - 1;
    }

    crs->x += p.delta_x;
    crs->y -= p.delta_y;

    redraw = true;
  }
  switch(gamestate) {
    case SINGLE: { // do nothing in single player mode
      break;
    }
    case MULTI: {
      // draw bubble on cursor that lasts a bit of time
      if(p.bytes[0] & LEFT_BUTTON) {
        if(((int)crs->x-150 < 0) || (int)crs->y-70 < 0) {
          break;
        }
        if(bubble == NULL) {
          // create animated sprite from bubble, bubble_small and bubble_big
          xpm_map_t img[3] = {bubble_xpm, bubble_small_xpm, bubble_big_xpm};

          bubble = create_animated_sprite("bubble", img, 3, crs->x-150, crs->y-70, 10);
        }
      }
      break;
    }
    case MAIN_MENU: {
      if(p.bytes[0] & LEFT_BUTTON) {
        gamestate = handle_click_main_menu(crs->x, crs->y);
      }
      break;
    }
    case GAME_OVER: {
      if(p.bytes[0] & LEFT_BUTTON) {
        gamestate = handle_click_game_over(crs->x, crs->y);
      }
      break;
    }
    default:
      break;
  }

}
