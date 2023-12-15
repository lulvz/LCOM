#include <lcom/lab2.h>
#include <lcom/lcf.h>

#include <stdbool.h>
#include <stdint.h>

#include "i8254.h"

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab2/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab2/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(timer_test_read_config)(uint8_t timer, enum timer_status_field field) {
  uint8_t st = 0x0;

  timer_get_conf(timer, &st);
  timer_display_conf(timer, st, field);

  return OK;
}

int(timer_test_time_base)(uint8_t timer, uint32_t freq) {
  timer_set_frequency(timer, freq);

  return OK;
}

// import variables from timer.c
extern long counter;

int(timer_test_int)(uint8_t time) {
  int ipc_status;
  message msg;
  uint32_t irq_set;
  uint8_t bit_no;

  if (timer_set_frequency(0, 60) != OK) {
    printf("Error in timer_set_frequency\n");
    return 1;
  }

  if (timer_subscribe_int(&bit_no) != OK) {
    printf("Error in timer_subscribe_int\n");
    return 1;
  }

  irq_set = BIT(bit_no);

  while (counter < time * 60) {
    /* Get a request message. */
    int r;
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != OK) {
      printf("Error in driver_receive: %d", r);
      continue;
    }

    if (is_ipc_notify(ipc_status)) { /* received notification */
      if (_ENDPOINT_P(msg.m_source) == HARDWARE) {
        if (msg.m_notify.interrupts & irq_set) { /* subscribed interrupt */
          timer_int_handler();
          if (counter % 60 == 0) {
            timer_print_elapsed_time();
          }
        }
      }
    }
  }

  if (timer_unsubscribe_int() != OK) {
    printf("Error in timer_unsubscribe_int\n");
    return 1;
  }

  return OK;
}
