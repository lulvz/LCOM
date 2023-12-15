#include <lcom/lcf.h>

#include <stdint.h>

#include "i8042.h"
#include "keyboard.h"

#define WAIT_KBC 20000
#define TRIES 5

/// @brief Variable used for storing the hook_id of the keyboard interrupts.
int hook_id_keyboard = 1; // from 0 to 31

/// @brief Variable used for storing the scancode read from the output buffer.
/// If the data_keyboard_variable stores 0xe000, it means that the full scancode will be completed in the next interrupt.
uint16_t data_keyboard = 0x0000;

// 0000 0000 - shift(0), caps(1), ctrl(2), alt(3)
static uint8_t modifier_keys_mask = 0x00; // 0x00 means no modifier keys are pressed

keycode_lu_t make_keycodes_normal[] = {
	{1,	53,	0x29,	0x0e,	0x0e,	"`~",},
	{2,	30,	0x02,	0x16,	0x16,	"1!",},
	{3,	31,	0x03,	0x1e,	0x1e,	"2@",},
	{4,	32,	0x04,	0x26,	0x26,	"3#",},
	{5,	33,	0x05,	0x25,	0x25,	"4$",},
	{6,	34,	0x06,	0x2e,	0x2e,	"5%",},
	{7,	35,	0x07,	0x36,	0x36,	"6^",},
	{8,	36,	0x08,	0x3d,	0x3d,	"7&",},
	{9,	37,	0x09,	0x3e,	0x3e,	"8*",},
	{10,	38,	0x0a,	0x46,	0x46,	"9(",},
	{11,	39,	0x0b,	0x45,	0x45,	"0)",},
	{12,	45,	0x0c,	0x4e,	0x4e,	"-_",},
	{13,	46,	0x0d,	0x55,	0x55,	"=+",},
  // 15,	42,	0x0e,	0x66,	0x66,	"Backspace",

	// 16,	43,	0x0f,	0x0d,	0x0d,	"Tab",
	{17,	20,	0x10,	0x15,	0x15,	"qQ",},
	{18,	26,	0x11,	0x1d,	0x1d,	"wW",},
	{19,	8,	0x12,	0x24,	0x24,	"eE",},
	{20,	21,	0x13,	0x2d,	0x2d,	"rR",},
	{21,	23,	0x14,	0x2c,	0x2c,	"tT",},
	{22,	28,	0x15,	0x35,	0x35,	"yY",},
	{23,	24,	0x16,	0x3c,	0x3c,	"uU",},
	{24,	12,	0x17,	0x43,	0x43,	"iI",},
	{25,	18,	0x18,	0x44,	0x44,	"oO",},
	{26,	19,	0x19,	0x4d,	0x4d,	"pP",},
	{27,	47,	0x1a,	0x54,	0x54,	"[{",},
	{28,	48,	0x1b,	0x5b,	0x5b,	"]}",},
	{29,	49,	0x2b,	0x5d,	0x5c,	"\\|",},

	{31,	04,	0x1e,	0x1c,	0x1c,	"aA",},
	{32,	22,	0x1f,	0x1b,	0x1b,	"sS",},
	{33,	7,	0x20,	0x23,	0x23,	"dD",},
	{34,	9,	0x21,	0x2b,	0x2b,	"fF",},
	{35,	10,	0x22,	0x34,	0x34,	"gG",},
	{36,	11,	0x23,	0x33,	0x33,	"hH",},
	{37,	13,	0x24,	0x3b,	0x3b,	"jJ",},
	{38,	14,	0x25,	0x42,	0x42,	"kK",},
	{39,	15,	0x26,	0x4b,	0x4b,	"lL",},
	{40,	51,	0x27,	0x4c,	0x4c,	";:",},
	{41,	52,	0x28,	0x52,	0x52,	"'\"",},

	// 43,	40,	0x1c,	0x5a,	0x5a,	"Enter",

  {46,	29,	0x2c,	0x1a,	0x1a,	"zZ",},
  {47,	27,	0x2d,	0x22,	0x22,	"xX",},
  {48,	6,	0x2e,	0x21,	0x21,	"cC",},
  {49,	25,	0x2f,	0x2a,	0x2a,	"vV",},
  {50,	5,	0x30,	0x32,	0x32,	"bB",},
  {51,	17,	0x31,	0x31,	0x31,	"nN",},
  {52,	16,	0x32,	0x3a,	0x3a,	"mM",},
  {53,	54,	0x33,	0x41,	0x41,	",<",},
  {54,	55,	0x34,	0x49,	0x49,	".>",},
  {55,	56,	0x35,	0x4a,	0x4a,	"/?",},

	// 61,	44,	0x39,	0x29,	0x29,	"space",

	// 64,	228,	0xe01d,	0xe014,	0x58,	"RCtrl",

	// 76,	76,	0xe053,	0xe071,	0x64,	"Delete",
	// 79,	80,	0xe04b,	0xe06b,	0x61,	"Left",
	// 83,	82,	0xe048,	0xe075,	0x63,	"Up",
	// 84,	81,	0xe050,	0xe072,	0x60,	"Down",
	// 89,	79,	0xe04d,	0xe074,	0x6a,	"Right",

  // upper row
	// 110,	41,	0x01,	0x76,	0x08,	"Esc",
};

keycode_sp_t modifier_keycodes[] = {
  {0x002a, 0x00aa, SHIFT},
  {0x0036, 0x00b6, SHIFT},
  {0x003a, 0x00ba, CAPS_LOCK},
  {0x001d, 0x009d, CTRL},
  {0x0038, 0x00b8, ALT},
  {0xe038, 0xe0b8, ALT}
};

/// @brief Variable used for storing the hook_id of the keyboard interrupts.
int hook_id_mouse = 2;

uint8_t scancode_mouse;
uint8_t bytes[3];
struct packet p;

uint8_t byte_counter = 0;
bool building = false;

// KEYBOARD //
/// @brief Subscribes and enables keyboard interrupts.
/// @param bit_no Address of memory to be initialized with the bit number to be set in the mask. Returned upon an interrupt.
/// @return Return 0 upon success and non-zero otherwise.
int keyboard_subscribe_int(uint8_t *bit_no) {

  *bit_no = hook_id_keyboard;

  if (sys_irqsetpolicy(KEYBOARD_IRQ, (IRQ_EXCLUSIVE | IRQ_REENABLE), &hook_id_keyboard) != OK) {
    printf("Error in keyboard sys_irqsetpolicy\n");
    return 1;
  }

  return OK;
}

/// @brief Unsubscribes keyboard interrupts.
/// @return Return 0 upon success and non-zero otherwise.
int keyboard_unsubscribe_int() {
  if (sys_irqrmpolicy(&hook_id_keyboard) != OK) {
    printf("Error in keyboard sys_irqrmpolicy\n");
    return 1;
  }

  return OK;
}

/// @brief Reads the status register of the keyboard, and checks if the output buffer is full.
/// @return Return 0 upon success and non-zero otherwise.
int kbc_read_data() {
  uint8_t stat;
  uint8_t tmp_data;

  // read status
  if (util_sys_inb(KBC_ST_REG, &stat) != OK) {
    printf("Error in util_sys_inb\n");
    return 1;
  }

  /* check if the 8042 output buffer is empty */
  if (stat & KBC_ST_OBF) {
    if (util_sys_inb(KBC_OUT_BUF, &tmp_data)) {
      printf("Error in util_sys_inb\n");
      return 1;
    }
    if ((stat & (KBC_PAR_ERR | KBC_TO_ERR)) == 0) {
      // first byte
      if (tmp_data == 0xe0) {
        data_keyboard = 0xe000;
        return OK;
      }
      // second byte
      if (data_keyboard == 0xe000) {
        data_keyboard |= tmp_data;
        return OK;
      }
      // normal 1 byte scancode
      data_keyboard = (0x0000 | tmp_data);
      return OK;
    }
    else {
      printf("Error in kbc_read_data: %d\n", stat);
      return 1;
    }
  }
  return 1;
}

/// @brief Keyboard interrupt handler.
void(kbc_ih)() {
  kbc_read_data();
}

/// @brief Updates the modifier keys mask.
void kbc_update_modifier_keys() {
  // data not yet constructed
  if (data_keyboard == 0xe000) {
    return;
  }
  // check for shift makecode
  if ((data_keyboard == modifier_keycodes[0].makecode) || (data_keyboard == modifier_keycodes[1].makecode)) {
    modifier_keys_mask |= BIT(SHIFT);
    return;
  } 
  // check for shift breakcode
  else if ((data_keyboard == modifier_keycodes[0].breakcode) || (data_keyboard == modifier_keycodes[1].breakcode)) {
    modifier_keys_mask &= ~BIT(SHIFT);
    return;
  }
  // check for caps lock makecode (caps lock is a toggle so we need to check if it's already on, this is done by using xor)
  else if (data_keyboard == modifier_keycodes[2].makecode) {
    modifier_keys_mask ^= BIT(CAPS_LOCK);
    return;
  }
  // check for ctrl makecode
  else if (data_keyboard == modifier_keycodes[3].makecode) {
    modifier_keys_mask |= BIT(CTRL);
    return;
  }
  // check for ctrl breakcode
  else if (data_keyboard == modifier_keycodes[3].breakcode) {
    modifier_keys_mask &= ~BIT(CTRL);
    return;
  }
  // check for alt makecode
  else if ((data_keyboard == modifier_keycodes[4].makecode) || (data_keyboard == modifier_keycodes[5].makecode)) {
    modifier_keys_mask |= BIT(ALT);
    return;
  }
  // check for alt breakcode
  else if ((data_keyboard == modifier_keycodes[4].breakcode) || (data_keyboard == modifier_keycodes[5].breakcode)) {
    modifier_keys_mask &= ~BIT(ALT);
    return;
  }
}

/// @brief Returns the upper_lower ascii char[2] corresponding to the scancode.
/// @param scancode Scancode to be converted.
/// @return Return the ascii char[2] corresponding to the scancode.
char* kbc_scancode_to_ascii(uint16_t scancode) {
  for(int i = 0; i < 47; i++) {
    if(scancode == make_keycodes_normal[i].set1) {
      return make_keycodes_normal[i].lower_upper;
    }
  }
  return NULL; // not found
}

/// @brief Returns an event_t struct with the action and char corresponding to the scancode.
/// The manager (supervisor) will redirect the event to the correct handler (game/menu).
/// This was build based on the event driven design pattern ie: https://www.devonblog.com/software-development/event-driven-design-pattern/;
/// @return Return an event_t struct with the action and char corresponding to the scancode.
event_t kbc_get_event() {
  kbc_update_modifier_keys();

  // data still not cosntructed
  if(data_keyboard == 0xe000) {
    event_t tmp = {NO_ACTION, 0x00};
    return tmp;
  }

  if(modifier_keys_mask & BIT(CTRL)) {
    if(data_keyboard == 0x000e) { // ctrl backspace
      event_t tmp = {DELETE_WORD_BACK, 0x00};
      return tmp;
    }
  } else if(modifier_keys_mask & BIT(ALT)) {
    if(data_keyboard == 0x000e) { // alt backspace
      event_t tmp = {DELETE_WORD_BACK, 0x00};
      return tmp;
    }
  }

  // TODO: SPECIAL KEYS
  if(data_keyboard == ESC_BREAK) {
    event_t tmp = {EXIT_PROGRAM, 0x00};
    return tmp;
  } else if (data_keyboard == 0x0039) {
    event_t tmp = {SPACE, 0x00};
    return tmp;
  } else if (data_keyboard == 0x000e) {
    event_t tmp = {DELETE_CHAR_BACK, 0x00};
    return tmp;
  } else if (data_keyboard == 0x001c) {
    event_t tmp = {ENTER, 0x00};
    return tmp;
  } else if (data_keyboard == 0x000f) {
    event_t tmp = {TAB, 0x00};
    return tmp;
  }

  // if it's a normal key being read
  char ascii_c = 0x00;
  // check if caps lock is on
  if (modifier_keys_mask & BIT(CAPS_LOCK)) {
    // check if shift is on
    if (modifier_keys_mask & BIT(SHIFT)) {
      char* res = kbc_scancode_to_ascii(data_keyboard);
      if(res == NULL) {
        event_t tmp = {NO_ACTION, 0x00};
        return tmp;
      }
      ascii_c = res[0];
    }
    // shift is off
    else {
      char* res = kbc_scancode_to_ascii(data_keyboard);
      if(res == NULL) {
        event_t tmp = {NO_ACTION, 0x00};
        return tmp;
      }
      ascii_c = res[1];
    }
  }
  // caps lock is off
  else {
    // check if shift is on
    if (modifier_keys_mask & BIT(SHIFT)) {
      char* res = kbc_scancode_to_ascii(data_keyboard);
      if(res == NULL) {
        event_t tmp = {NO_ACTION, 0x00};
        return tmp;
      }
      ascii_c = res[1];
    }
    // shift is off
    else {
      char* res = kbc_scancode_to_ascii(data_keyboard);
      if(res == NULL) {
        event_t tmp = {NO_ACTION, 0x00};
        return tmp;
      }
      ascii_c = res[0];
    }
  }
  event_t tmp = {PRINT_CHAR, ascii_c};
  return tmp;
}

/// @brief Issues a command to the KBC.
/// @param cmd Command to be issued.
/// @return Return 0 upon success and non-zero otherwise.
int kbc_issue_command(uint8_t cmd) {
  uint8_t stat = 0;
  int retry_counter = TRIES;

  // loops while the input buffer is not empty
  while (retry_counter > 0) {
    // check if status is readable
    if (util_sys_inb(KBC_ST_REG, &stat) != OK) {
      printf("Error in util_sys_inb\n");
      return 1;
    }

    // check if the input buffer is empty
    if ((stat & KBC_ST_IBF) == 0) {
      sys_outb(KBC_CMD_REG, cmd); /* no args command */
      return OK;
    }

    // delay
    tickdelay(micros_to_ticks(WAIT_KBC));
    retry_counter--;
  }

  return 1;
}

/// @brief Enables the keyboard.
/// @return Return 0 upon success and non-zero otherwise.
int kbd_enable() {
  uint8_t command_byte;

  if (sys_outb(KBC_CMD_REG, READ_CMD_BYTE) != OK) {
    printf("Error in sys_outb\n");
    return 1;
  }

  if (util_sys_inb(KBC_OUT_BUF, &command_byte) != OK) {
    printf("Error in util_sys_inb\n");
    return 1;
  }

  // set bit 0 to 1
  command_byte |= BIT(0);

  if (sys_outb(KBC_CMD_REG, KBC_OUT_BUF) != OK) {
    printf("Error in sys_outb\n");
    return 1;
  }

  if (sys_outb(WRITE_CMD_BYTE, command_byte) != OK) {
    printf("Error in sys_outb\n");
    return 1;
  }
  return OK;
}

// MOUSE //
/// @brief Subscribes and enables mouse interrupts.
/// @param bit_no Address of memory to be initialized with the bit number to be set in the mask returned upon an interrupt.
/// @return Return 0 upon success and non-zero otherwise.
int(mouse_subscribe_int)(uint8_t *bit_no) {
  *bit_no = hook_id_mouse;

  if(sys_irqsetpolicy(KBD_AUX_IRQ, (IRQ_REENABLE | IRQ_EXCLUSIVE), &hook_id_mouse) != OK) {
    printf("Error setting policy\n");
    return 1;
  }

  return OK; 
}

/// @brief Writes a command to the mouse.
/// @param cmd Command to be written.
/// @param response Address of memory to be initialized with the response of the command.
/// @return Return 0 upon success and non-zero otherwise.
int (mouse_write_cmd)(int cmd, uint8_t* response) {
  uint8_t stat;
  int retry_counter = TRIES;

  while(retry_counter > 0) {
    if(util_sys_inb(KBC_CMD_REG, &stat) != OK) {
      printf("Error in util_sys_inb\n");
      return 1;
    }

    if((stat & KBC_ST_IBF) == 0) {
      if(sys_outb(WRITE_CMD_BYTE, cmd) != OK) {
        printf("Error in sys_outb\n");
        return 1;
      }
      if(util_sys_inb(KBC_OUT_BUF, response) != OK) {
        printf("Error in util_sys_inb\n");
        return 1;
      }
      return OK;
    }

    retry_counter--;
  }

  return 1;
}

/// @brief Enables mouse data reporting.
/// @return Return 0 upon success and non-zero otherwise.
int(mouse_enable_data_reporting_custom)() {
  // return mouse_enable_data_reporting();
  uint8_t response;
  int attempts = 2;

  do {
    if (sys_outb(KBC_CMD_REG, 0xD4) != OK) {
      printf("Error in sys_outb\n");
      return 1;
    }
    if (mouse_write_cmd(0xF4, &response) != OK) {
      printf("Error in mouse_write_cmd\n");
      return 1;
    }

    // ACK
    if (response == 0xFA) {
      return OK;
    }

    // NACK
    else if (response == 0xFE) {
      attempts--;
    }

    // ERROR
    else if (response == 0xFC) {
      printf("Error in mouse_enable_data_reporting\n");
      return 1;
    }
  } while (attempts > 0);

  return 1;
}

/// @brief Unsubscribes mouse interrupts.
/// @return Return 0 upon success and non-zero otherwise.
int(mouse_unsubscribe_int)() {
  if(sys_irqrmpolicy(&hook_id_mouse) != OK) {
    printf("Error unsubscribing mouse interrupts\n");
    return 1;
  }

  return OK;
}

/// @brief Checks if the output buffer is full.
/// @return Return true if the output buffer is full, false otherwise.
bool(check_output_buffer_full)() {
  uint8_t stat;
  if(util_sys_inb(KBC_ST_REG, &stat) != OK) {
    return false;
  }
  if((stat & KBC_ST_OBF) && (stat & BIT(5))) {
    if((stat & (KBC_PAR_ERR | KBC_TO_ERR)) == 0) {
      return true;
    }
    return false;
  }
  return false;
}

/// @brief Mouse interrupt handler.
void (mouse_ih)() {
  // ckeck output buffer try to receive using util_sys_inb from the OUT_BUF
    if (check_output_buffer_full()) {
      if (util_sys_inb(KBC_OUT_BUF, &scancode_mouse) != OK) {
        // scancode_mouse = 0x0;
        printf("Error in util_sys_inb in mouse_ih\n");
        return;
      }
    }
}

/// @brief Builds a packet from the bytes received from the mouse.
/// @param p Address of memory to be initialized with the packet.
void build_packet(struct packet * p) {
  p->bytes[0] = bytes[0];
  p->bytes[1] = bytes[1];
  p->bytes[2] = bytes[2];

  p->lb = bytes[0] & LEFT_BUTTON;
  p->rb = bytes[0] & RIGHT_BUTTON;
  p->mb = bytes[0] & MIDDLE_BUTTON;

  if (bytes[0] & MSB_X_DELTA) {
    p->delta_x = 0xff00 | bytes[1];
  } else {
    p->delta_x = bytes[1];
  }

  if (bytes[0] & MSB_Y_DELTA) {
    p->delta_y = 0xff00 | bytes[2];
  } else {
    p->delta_y = bytes[2];
  }

  p->x_ov = bytes[0] & X_OVERFLOW;
  p->y_ov = bytes[0] & Y_OVERFLOW;
}

/// @brief Checks if the mouse is moving.
/// @return Return true if the mouse is moving, false otherwise.
bool (mouse_stream_handler)() {
  mouse_ih();
  if(scancode_mouse & BIT(3)) {
    building = true;
  }
  if(building) {
    bytes[byte_counter] = scancode_mouse;
    byte_counter++;
  }
  
  if(byte_counter == 3) {
    build_packet(&p);
    byte_counter = 0;
    building = false;
    return true;
  }
  return false;
}
