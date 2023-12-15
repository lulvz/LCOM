#ifndef MANAGER_H
#define MANAGER_H

int subscribe_ints(void);

int init_all(void);

int unsubscribe_ints(void);

int end_all(void);

int main_game_loop(void);

void timer_int(void);
void keyboard_int(void);
void mouse_int(void);

#endif
