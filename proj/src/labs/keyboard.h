#ifndef KEYBOARD_H 
#define KEYBOARD_H 

#include <stdint.h>
#include <lcom/lcf.h>

// KEYBOARD //

// this was copied from https://www.win.tue.nl/~aeb/linux/kbd/table.h
// (it was slightly addapted to our use case)
typedef struct {
	unsigned int position, usb;

  uint16_t set1;

  uint16_t set2, set3;
	char lower_upper[2];		/* keycap on a standard US keyboard */
} keycode_lu_t;

// used to identify the bits in the special keys mask
enum special_keys {
  SHIFT, // works for both left and right shift (BIT(0))
  CAPS_LOCK, // a toggle bit for caps lock (BIT(1))
  CTRL, // right ctrl is used in vm to release mouse (BIT(2))
  ALT, // works for both left and right alt (BIT(3))
};

// we need the break code for special keys such as shift and ctrl
typedef struct {
  uint16_t makecode;
  uint16_t breakcode;
  enum special_keys key;
} keycode_sp_t;

enum actions {
  NO_ACTION, // no action to be performed
  PRINT_CHAR, // add the character to the cursor position
  SPACE, // move onto next word
  DELETE_CHAR_BACK, // delete the character behind the cursor (backspace)
  DELETE_CHAR_FRONT, // delete the character in front of the cursor (delete)
  DELETE_WORD_BACK, // delete the word behind the cursor (ctrl/alt + backspace)
  DELETE_WORD_FRONT, // delete the word in front of the cursor (ctrl/alt + delete)

  MOVE_BACK, // move the cursor one position back (left arrow)
  MOVE_FORWARD, // move the cursor one position forward (right arrow)
  EXIT_PROGRAM, // exit the program (esc)

  ENTER, // for saving the name in the game over screen
  TAB // for restarting the game quicker in the game over screen
};

typedef struct {
  enum actions action; // the action to be performed
  char c; // the character to be printed, if any
} event_t;

int keyboard_subscribe_int(uint8_t *bit_no);

int keyboard_unsubscribe_int();

// takes the scancode and updates the mask of modifier keys accordingly
void kbc_update_modifier_keys();

int kbc_read_data();

// kbc_read_data() must be executed before to update variables
event_t kbc_get_event();

int kbc_issue_command(uint8_t cmd);

int kbd_enable();

// MOUSE //
int(mouse_subscribe_int)(uint8_t *bit_no);

int(mouse_enable_data_reporting_custom)();

int(mouse_unsubscribe_int)();

bool(check_output_buffer_full)();

void(build_packet)(struct packet * p);

bool(mouse_stream_handler)();

#endif
