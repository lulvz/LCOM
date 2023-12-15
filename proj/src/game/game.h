#ifndef GAME_H
#define GAME_H

#include <lcom/lcf.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <dirent.h>

#include "../labs/keyboard.h"
#include "gamestate.h"
#include "gameover.h"

#define MAX_ROW_CHARS 66
#define MAX_ROWS 11
#define MAX_TOTAL_CHARS (MAX_ROW_CHARS * MAX_ROWS)

unsigned long get_game_counter();

int generate_race(char *sentence);

void increase_game_counter();

int advance_word();

int cmp_current_word();

void end_game();

void game_handle_event(event_t e);

// load selected race from races folder
void load_race(int race_no);

// loads a random race into the race_string variable
void load_random_race();

void reset_all();

#endif
