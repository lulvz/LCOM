#ifndef GAMEOVER_H
#define GAMEOVER_H

#include "game.h"
#include "gamestate.h"
#include "../labs/rtc.h"

typedef struct {
  char name[100];
  float wpm;
  char date[100];
} highscore_t;

bool is_score_generated();
unsigned int get_final_correct_words();
unsigned int get_total_words();
unsigned int get_final_correct_chars();
unsigned int get_total_chars();
float get_final_wpm();
char* get_user_name();

uint16_t (*get_button_positions_game_over())[4];

gamestate_t handle_click_game_over(uint16_t x, uint16_t y);

int generate_score();

int get_top10_highscores(highscore_t *highscores);

void game_over_handle_event(event_t e);

void reset_score();

#endif
