#include "game.h"

#define RACE_LOCATION "/home/lcom/labs/proj/src/races/"
#define MAX_RACES 200

extern gamestate_t gamestate;

char rows[MAX_ROWS][MAX_ROW_CHARS+1];
char full_sentence[MAX_TOTAL_CHARS+MAX_ROWS] = {0}; // we sum max_rows because of the \0 at the end of each row
int current_word_idx = 0; // index of the current word being typed in the full_sentence array
char user_word[100]; // word used to store the current word being typed

char final_user_sentence[MAX_TOTAL_CHARS*3] = {0}; // final sentence typed by the user to be compared to the full_sentence

static unsigned long game_counter = 0;

/// @brief Gets the game counter.
/// @return Returns the game counter.
unsigned long get_game_counter() {
  return game_counter;
}

/// @brief Generates a race given a specific input file.
/// @param sentence Text to be generated.
/// @return Returns the number of rows used.
int generate_race(char *sentence) {
  // the sentence array should be divided into the rows matrix, with each row having a maximum of 88 characters
  // the function should return the number of rows used

  // fill out the rows array
  int i = 0;
  for (; i < MAX_ROWS; i++) {
    for (int j = 0; j < MAX_ROW_CHARS; j++) {
      if (sentence[i * MAX_ROW_CHARS + j] == '\0') {
        rows[i][j] = '\0';
        return i + 1;
      }
      
      rows[i][j] = sentence[i * MAX_ROW_CHARS + j];
      if(j == MAX_ROW_CHARS - 1) {
        rows[i][j+1] = '\0';
      }
    }
  }
  return i + 1;
}

/// @brief Increments the game counter.
void increase_game_counter() {
  game_counter++;
}

/// @brief Advances to the next word.
/// @return Returns 1 if the end of the sentence is reached, 0 otherwise.
int advance_word() {
  // also set the user_word to the next word
  // the current_word_idx is at the start of the last word typed
  // the next word starts at the end of the last word typed + 1
  int tmp = current_word_idx;

  // advance the current_word_idx until either a space of the end of the sentence is reached
  while (full_sentence[tmp] != ' ' && full_sentence[tmp] != '\0') {
    tmp++;
  }
  if (full_sentence[tmp] == '\0') {
    // if the end of the sentence is reached, append the word to the final_user_sentence and return 1
    strcat(final_user_sentence, user_word);
    return 1;
  }
  tmp++;
  // append the word to the final_user_sentence and a space
  strcat(final_user_sentence, user_word);
  strcat(final_user_sentence, " ");
  current_word_idx = tmp;

  return 0;
}

/// @brief Compares the current word being typed to the word in the sentence.
/// @return Returns 0 if the word is correct, 1 otherwise.
int cmp_current_word() {
  int i = 0;
  while ((user_word[i] != '\0') && (full_sentence[current_word_idx + i] != '\0')) {
    if (user_word[i] != full_sentence[current_word_idx + i]) {
      return 1;
    }
    i++;
  }

  return OK;
}

/// @brief Ends the game.
/// Generates the score and resets all the variables.
void end_game() {
  generate_score(full_sentence, final_user_sentence, game_counter);

  reset_all();
  gamestate = GAME_OVER;
}

/// @brief Handles game events.
/// @param e Event to be handled.
void game_handle_event(event_t e) {
  printf("action: %d\n", e.action);

  if(e.action == NO_ACTION) return;


  if (e.action == EXIT_PROGRAM) {
    gamestate = MAIN_MENU;
    return;
  }

  if (e.action == DELETE_CHAR_BACK) {
    // remove last character from user_word
    if (strlen(user_word) > 0) {
      user_word[strlen(user_word) - 1] = '\0';
    }
  } else if (e.action == SPACE) {
    // advance to the next word
    if (advance_word()) {
      // if the end of the sentence is reached, end the game
      printf("end of sentence reached\n");
      end_game();
      return;
    }
    // reset the user_word
    user_word[0] = '\0';
  } else if (e.action == PRINT_CHAR) {
    // append the letter to the user_word
    if (strlen(user_word) < 100) {
      user_word[strlen(user_word) + 1] = '\0';
      user_word[strlen(user_word)] = e.c;
    }
  } else if (e.action == DELETE_WORD_BACK) {
    printf("delete word back\n");
    // reset user_word
    user_word[0] = '\0';
  }  
}

/// @brief Loads a race specified in the argument.
/// @param race_no Number of the race to be loaded.
void load_race(int race_no) {
  reset_all();
  reset_score();
  // open file in format r<race_no>.race in races folder
  if (race_no < 0)
    return;

  char filename[40];
  sprintf(filename, RACE_LOCATION "r%d.race", race_no);

  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    printf("Error opening file %s\n", filename);
    return;
  }

  fseek(fp, 0, SEEK_END);
  long fsize = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  if (fsize > MAX_TOTAL_CHARS) {
    printf("File %s is too big!\n", filename);
    return;
  }

  size_t b = fread(full_sentence, sizeof(char), fsize, fp);
  full_sentence[b] = '\0';

  generate_race(full_sentence);


  fclose(fp);
}

/// @brief Gets the number of races available.
/// @return Returns the number of races available.
int get_num_races() {
  int count = 0;

  DIR *dir = opendir(RACE_LOCATION);
  if (dir != NULL) {
    struct dirent *e;
    while ((e = readdir(dir)) != NULL) {
      if (e->d_type == DT_REG && strstr(e->d_name, ".race") != NULL) {
        count++;
      }
    }
    closedir(dir);
  } else {
    printf("Error opening directory\n");
  }

  return count;
}

/// @brief Loads a random race to be played.
void load_random_race() {
  int num_races = get_num_races();
  printf("%d\n", num_races);
  if (num_races == 0) {
    printf("No races found\n");
    return;
  }

  int random_race = rand() % num_races;

  load_race(random_race);
}

/// @brief Resets all the variables.
void reset_all() {
  final_user_sentence[0] = '\0';
  full_sentence[0] = '\0';
  current_word_idx = 0;
  user_word[0] = '\0';
  for(int i = 0; i<MAX_ROWS; i++) {
    rows[i][0] = '\0';
  }
  game_counter = 0;
}
