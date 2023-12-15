#ifndef KEYBOARD_H 
#define KEYBOARD_H  
#include <stdint.h>

int keyboard_subscribe_int(uint8_t *bit_no);

int keyboard_unsubscribe_int();

int kbc_read_data();

int kbc_issue_command(uint8_t cmd); 

void kbc_handler();

int kbd_enable();

#endif
