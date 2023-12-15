#include <lcom/lcf.h>

#include <lcom/lab3.h>

#include <stdbool.h>
#include <stdint.h>

#include <keyboard.h>
#include "i8042.h"

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab3/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab3/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

extern uint8_t data;

int(kbd_test_scan)() {
  
  uint8_t bit_no = 1;
  uint8_t scancodes[2] = {0, 0};
  bool readAgain = false;

  if (keyboard_subscribe_int(&bit_no) != OK) {
    printf("error in keyboard_subscribe_int\n");
    return 1;
  }

  uint32_t irq_set = BIT(bit_no);

  int ipc_status;
  message msg;

  while(data != ESC_BREAK) {
    // check if there is a message to get

    int r;
    if((r = driver_receive(ANY, &msg, &ipc_status)) != OK) {
      printf("error in driver_receive: %d\n", r);
      continue;
    }

    if(is_ipc_notify(ipc_status)) {
      if(_ENDPOINT_P(msg.m_source) == HARDWARE) {
        if(msg.m_notify.interrupts & irq_set) {
          kbc_ih();
          if(readAgain) {
            // runs after the first byte of a two byte scancode
            scancodes[1] = data;
            readAgain = false;
            bool make = !(data & BREAK);
            kbd_print_scancode(make, 2, scancodes);
          } else {
            scancodes[0] = data;
            if(data == SCANCODE_DOUBLE) {
              readAgain = true;
            } else {
              bool make = !(data & BREAK);
              kbd_print_scancode(make, 1, scancodes);
            }
          }
        }
      }
    }
  }

  keyboard_unsubscribe_int();

  return OK;
}

int(kbd_test_poll)() {
  uint8_t scancodes[2] = {0, 0};
  while(data != ESC_BREAK) {
    kbc_ih();
    if(data == SCANCODE_DOUBLE) {
      scancodes[0] = data;
      kbc_ih();
      scancodes[1] = data;
      bool make = !(data & BREAK);
      kbd_print_scancode(make, 2, scancodes);
    } else {
      scancodes[0] = data;
      bool make = !(data & BREAK);
      kbd_print_scancode(make, 1, scancodes);
    }
    tickdelay(micros_to_ticks(20000));
  }
  if(kbd_enable() != OK) {
    return 1;
  }
  return OK;
}

extern long counter;

int(kbd_test_timed_scan)(uint8_t n) {
  uint8_t idle = n;
  uint8_t bit_no_keyboard, bit_no_timer;

  if (keyboard_subscribe_int(&bit_no_keyboard) != OK) {
    printf("error in keyboard_subscribe_int\n");
    return 1;
  }

  if (timer_subscribe_int(&bit_no_timer) != OK) {
    printf("error in timer_subscribe_int\n");
    return 1;
  }

  timer_set_frequency(0, 60);

  // get the masks from the bit numbers
  uint32_t irq_set_keyboard = BIT(bit_no_keyboard);
  uint32_t irq_set_timer = BIT(bit_no_timer);

  int r;
  int ipc_status;
  message msg;

  uint8_t scancodes[2] = {0, 0};
  bool readAgain = false;

  while(idle != 0 && data != ESC_BREAK) {

    // check if there is a message to get
    if((r = driver_receive(ANY, &msg, &ipc_status)) != OK) {
      printf("error in driver_receive: %d\n", r);
      continue;
    }

    if(is_ipc_notify(ipc_status)) {
      if(_ENDPOINT_P(msg.m_source) == HARDWARE) {
        if(msg.m_notify.interrupts & irq_set_keyboard) {

          // keyboard interrupt handler
          kbc_ih();

          if(readAgain) {
            // runs after the first byte of a two byte scancode
            scancodes[1] = data;
            readAgain = false;
            bool make = !(data & BREAK);
            kbd_print_scancode(make, 2, scancodes);
          } else {
            scancodes[0] = data;
            if(data == SCANCODE_DOUBLE) {
              readAgain = true;
            } else {
              bool make = !(data & BREAK);
              kbd_print_scancode(make, 1, scancodes);
            }
          }

          idle = n;
        }
        if(msg.m_notify.interrupts & irq_set_timer) {
          // timer interrupt handler
          timer_int_handler();
          if(counter % 60 == 0 && counter != 0) {
            idle--;
          }
        }
      }
    }
  }

  keyboard_unsubscribe_int();

  return OK;
}
