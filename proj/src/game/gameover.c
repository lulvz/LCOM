#include "gameover.h"

#define HIGHSCORES_LOCATION "/home/lcom/labs/proj/src/highscores/"

extern gamestate_t gamestate;

bool generated = false;

static unsigned int correct_words = 0;
static unsigned int total_words = 0;
static unsigned int correct_chars= 0;
static unsigned int total_chars = 0;
static float final_wpm = 0;

static char user_name[100] = {0};

// button positions (x, y, width, height)
static uint16_t buttons[3][4] = {
  { 200, 700, 150, 80 }, // REPLAY BUTTON
  { 700, 700, 150, 80 }, // MENU BUTTON
};

// getter for x and y of the buttons
// return type uint16_t (*)[4], which is a pointer to an array of 4 uint16_t
// kinda confusing but its the only way that it works xd
uint16_t (*get_button_positions_game_over())[4] {
  return buttons;
}

/// @brief Checks if the score has been generated.
/// @return Returns true if the score has been generated and false otherwise.
bool is_score_generated() {
  return generated;
}

/// @brief Gets the final number of correct words typed by the player.
/// @return Returns the final number of correct words typed by the player.
unsigned int get_final_correct_words() {
  return correct_words;
}

/// @brief Gets the total number of words typed by the player.
/// @return Returns the total number of words typed by the player.
unsigned int get_total_words() {
  return total_words;
}

/// @brief Gets the total number of characters typed correctly by the player.
/// @return Returns the total number of characters typed correctly by the player.
unsigned int get_final_correct_chars() {
  return correct_chars;
}

/// @brief Gets the total number of characters typed by the player.
/// @return Returns the total number of characters typed by the player.
unsigned int get_total_chars() {
  return total_chars;
}

/// @brief Gets the final words per minute of the player.
/// @return Returns the final words per minute of the player.
float get_final_wpm() {
  return final_wpm;
}

/// @brief Gets the user name.
/// @return Returns the user name.
char* get_user_name() {
  return user_name;
}

/// @brief Handles and sets the correct gamestate based on where the user clicked.
/// @param x X coordinate of the click.
/// @param y Y coordinate of the click.
/// @return Returns the corresponding gamestate to switch to.
gamestate_t handle_click_game_over(uint16_t x, uint16_t y) {
  for (int i = 0; i < 3; i++) {
    if ((x >= buttons[i][0]) && (x <= buttons[i][0] + buttons[i][2]) &&
        (y >= buttons[i][1]) && (y <= buttons[i][1] + buttons[i][3])) {
      switch (i) {
        case 0: {
          load_random_race();
          return SINGLE;
        }
        case 1: {
          return MAIN_MENU;
        }
      }
    }
  }

  return GAME_OVER;
}

/// @brief Generates the player's final score.
/// The final score is based on the number of correct words, the number of correct characters, and the time taken to type the whole text file.
/// @param full_sentence Sentence generated by the game.
/// @param final_user_sentence Sentence typed by the player.
/// @param time_taken Time taken to type the whole text file.
/// @return Returns 0 upon success and non-zero otherwise.
int generate_score(char *full_sentence, char *final_user_sentence, unsigned long time_taken) {
  // just to be sure
  reset_score();

  char* full_words[MAX_TOTAL_CHARS/2];
  char* token = strtok(full_sentence, " ");
  while (token != NULL && total_words < (MAX_TOTAL_CHARS/2)) {
    full_words[total_words] = token;
    total_chars += strlen(token);
    token = strtok(NULL, " ");
    total_words++;
  }

  char* user_words[MAX_TOTAL_CHARS/2];
  unsigned int user_word_count = 0;
  token = strtok(final_user_sentence, " ");
  while (token != NULL && user_word_count < (MAX_TOTAL_CHARS/2)) {
    user_words[user_word_count] = token;
    token = strtok(NULL, " ");
    user_word_count++;
  }

  // Compare the words and characters
  for (unsigned int i = 0; (i < total_words) && (i < user_word_count); i++) {
    // get the amount of correct characters in the correct position in the word
    unsigned int correct_chars_in_word = 0;
    for (size_t j = 0; j < strlen(full_words[i]) && j < strlen(user_words[i]); j++) {
      if (full_words[i][j] == user_words[i][j]) {
        correct_chars_in_word++;
      }
    }
    correct_chars += correct_chars_in_word;

    if (strcmp(full_words[i], user_words[i]) == 0) {
        correct_words++;
    }
  }
  printf("correct words: %u out of %u\n", correct_words, total_words);
  printf("correct chars: %u out of %u\n", correct_chars, total_chars);

  printf("time taken: %ld\n", time_taken);
  
  // a word is generally considered 5 characters long, so we used 5 chars as a word
  // in this calculation
  final_wpm = ((float)correct_chars / 5.0f) / (((float)time_taken / 60.0f) / 60.0f);
  printf("final wpm: %f\n", final_wpm);

  generated = true; // to be used in manager to know if score is viable or not
  return OK;
}

/// @brief Returns the top 10 highscores.
/// @param highscores Array of highscores to be searched.
/// @return Returns the number of highscores found (10 if the number was 10 or more).
int get_top10_highscores(highscore_t *highscores) {
  // open the file
  FILE *fp;
  fp = fopen(HIGHSCORES_LOCATION "highscores.txt", "r");
  if(fp == NULL) {
    printf("Error opening file\n");
    return 0;
  }

  // read the file line by line
  char line[256];
  int i = 0;
  while(fgets(line, sizeof(line), fp)) {
    if(i >= 10) break; // only get the top 10 highscores
    // get the name from the line
    char *token = strtok(line, ";");
    strcpy(highscores[i].name, token);

    // get the wpm from the line
    token = strtok(NULL, ";");
    highscores[i].wpm = atof(token);

    // get the date from the line
    token = strtok(NULL, "\n");
    strcpy(highscores[i].date, token);

    i++;
  }

  // close the file
  fclose(fp);
  return i; // return amount of highscores found if not 10
}

/// @brief Checks if the user has a highscore already set.
/// @param name Name of the user.
/// @return Returns true if the user has a highscore set and false otherwise.
bool has_highscore(char* name) {
  // open the file
  FILE *fp;
  fp = fopen(HIGHSCORES_LOCATION "highscores.txt", "r");
  if(fp == NULL) {
    printf("Error opening file\n");
    return false;
  }

  // read the file line by line
  char line[256];
  while(fgets(line, sizeof(line), fp)) {
    // get the name from the line
    char *token = strtok(line, ";");
    if(strcmp(token, name) == 0) {
      // close the file
      fclose(fp);
      return true;
    }
  }

  // close the file
  fclose(fp);
  return false;
}

/// @brief Sorts the highscores by wpm.
/// @param highscores Array of highscores to be sorted.
/// @param size Size of the array.
void sort_by_wpm(highscore_t *highscores, int size) {
  // sort the array
  for(int j = 0; j < size; j++) {
    for(int k = 0; k < size - j - 1; k++) {
      if(highscores[k].wpm < highscores[k + 1].wpm) {
        highscore_t temp = highscores[k];
        highscores[k] = highscores[k + 1];
        highscores[k + 1] = temp;
      }
    }
  }
}

/// @brief Saves the highscore to the respective file (highscores.txt).
/// @param name Name of the user.
/// @param wpm WPM of the user in that highscore.
/// @param date Date of when the highscore was set.
void save_highscore(char *name, float wpm, char *date) {
  FILE* fp;
  if(has_highscore(name)) {
    // load whole file into memory, then update the highscore and sort the array
    fp = fopen(HIGHSCORES_LOCATION "highscores.txt", "r");
    if(fp == NULL) {
      printf("Error opening file\n");
      return;
    }

    // read the file line by line
    char line[256];
    int i = 0;
    // check how many highscores there are
    while(fgets(line, sizeof(line), fp)) {
      i++;
    }
    fclose(fp);

    // create an array of highscores
    highscore_t highscores[i];

    // open the file again
    fp = fopen(HIGHSCORES_LOCATION "highscores.txt", "r");
    if(fp == NULL) {
      printf("Error opening file\n");
      return;
    }

    // read the file line by line
    i = 0;
    while(fgets(line, sizeof(line), fp)) {
      // get the name from the line
      char *token = strtok(line, ";");
      strcpy(highscores[i].name, token);

      // get the wpm from the line
      token = strtok(NULL, ";");
      highscores[i].wpm = atof(token);

      // get the date from the line
      token = strtok(NULL, "\n");
      strcpy(highscores[i].date, token);

      i++;
    }

    // find the highscore with the same name and update it
    for(int j = 0; j < i; j++) {
      if(strcmp(highscores[j].name, name) == 0) {
        highscores[j].wpm = wpm;
        break;
      }
    }

    sort_by_wpm(highscores, i);

    fclose(fp);

    // open the file again
    fp = fopen(HIGHSCORES_LOCATION "highscores.txt", "w");
    if(fp == NULL) {
      printf("Error opening file\n");
      return;
    }

    for(int j = 0; j < i; j++) {
      fprintf(fp, "%s;%.2f;%s\n", highscores[j].name, highscores[j].wpm, highscores[j].date);
    }
  } else {
    // load whole file into memory, then add the new highscore and sort the array
    fp = fopen(HIGHSCORES_LOCATION "highscores.txt", "r");
    if(fp == NULL) {
      printf("Error opening file\n");
      return;
    }

    char line[256];
    int i = 0;
    // check how many highscores there are
    while(fgets(line, sizeof(line), fp)) {
      i++;
    }
    fclose(fp);
    if(i == 0) {
      // if there are no highscores, just add the new one
      fp = fopen(HIGHSCORES_LOCATION "highscores.txt", "w");
      if(fp == NULL) {
        printf("Error opening file\n");
        return;
      }
      fprintf(fp, "%s;%.2f;%s\n", name, wpm, date);
      fclose(fp);
      return;
    }

    highscore_t highscores[i + 1];
    fp = fopen(HIGHSCORES_LOCATION "highscores.txt", "r");
    if(fp == NULL) {
      printf("Error opening file\n");
      return;
    }

    i = 0;
    while(fgets(line, sizeof(line), fp)) {
      // get the name from the line
      char *token = strtok(line, ";");
      strcpy(highscores[i].name, token);

      // get the wpm from the line
      token = strtok(NULL, ";");
      highscores[i].wpm = atof(token);

      // get the date from the line
      token = strtok(NULL, "\n");
      strcpy(highscores[i].date, token);

      i++;
    }

    strcpy(highscores[i].name, name);
    highscores[i].wpm = wpm;
    strcpy(highscores[i].date, date);
    
    // sort the array
    sort_by_wpm(highscores, i + 1);

    // close the file
    fclose(fp);

    // open the file again
    fp = fopen(HIGHSCORES_LOCATION "highscores.txt", "w");
    if(fp == NULL) {
      printf("Error opening file\n");
      return;
    }

    // write the highscores to the file
    for(int j = 0; j < i + 1; j++) {
      fprintf(fp, "%s;%.2f;%s\n", highscores[j].name, highscores[j].wpm, highscores[j].date);
    }    
  }
  fclose(fp);
}

/// @brief Handles the events for the game over screen.
/// Handles the inputs for when the user is saving their highscore.
/// @param e Event to be handled.
void game_over_handle_event(event_t e) {
  if(e.action == NO_ACTION) return;

  if(e.action == EXIT_PROGRAM) {
    gamestate = MAIN_MENU;
    return;
  }

  if(e.action == DELETE_CHAR_BACK) {
    // remove last character from user_name
    if(strlen(user_name) > 0) {
      user_name[strlen(user_name) - 1] = '\0';
    }
  } else if(e.action == PRINT_CHAR) {
    if(e.c == ';') return; // ignore semicolons
    if(strlen(user_name) < 6) {
      user_name[strlen(user_name) + 1] = '\0';
      user_name[strlen(user_name)] = e.c;
    }
  } else if (e.action == DELETE_WORD_BACK) {
    user_name[0] = '\0';
  } else if(e.action == ENTER) {
    // save the score to the highscores file with the username, if its empty, use "anonymous"
    printf("Enter\n");
    if(strlen(user_name) == 0) {
      strcpy(user_name, "anon");
    }
    char date_time[20];
    get_full_date_time(date_time);
    save_highscore(user_name, final_wpm, date_time);
    // gamestate = MAIN_MENU;
  } else if(e.action == TAB) { // restart for efficient speed training
    load_random_race();
    gamestate = SINGLE;
  }
}

/// @brief Resets the score.
void reset_score() {
  correct_words = 0;
  total_words = 0;
  correct_chars = 0;
  total_chars = 0;
  final_wpm = 0;
  user_name[0] = '\0'; // clear user_name
  generated = false;
}