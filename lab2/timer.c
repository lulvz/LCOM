#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>

#include "i8254.h"

long counter = 0;
int hook_id_timer = 20;

int(timer_set_frequency)(uint8_t timer, uint32_t freq) {
  // obter freq divisor
  if (freq < 19 || freq > TIMER_FREQ) {
    printf("freq should be higher than 19 and lower than %d\n", TIMER_FREQ);
    return 1;
  }
  uint16_t freq_divisor = TIMER_FREQ / freq;

  // obter o byte lsb e o msb de acordo com o parametro passado

  uint8_t lsb = 0x0;
  util_get_LSB(freq_divisor, &lsb);

  uint8_t msb = 0x0;
  util_get_MSB(freq_divisor, &msb);

  // ler o status byte para manter o que naoo muda
  uint8_t st;
  timer_get_conf(timer, &st);

  uint8_t timer_cmd = (st & 0x0f) | timer << 6 | TIMER_LSB_MSB;

  sys_outb(TIMER_CTRL, timer_cmd);
  sys_outb(TIMER_CTRL, lsb);
  sys_outb(TIMER_CTRL, msb);

  // ler status byte, construir control word, escrever pra o contro lregister, fazer a formula matematica, escrever no timer o valor da formula
  return OK;
}

int(timer_subscribe_int)(uint8_t *bit_no) {
  *bit_no = hook_id_timer;

  if (sys_irqsetpolicy(TIMER0_IRQ, IRQ_REENABLE, &hook_id_timer) != OK) {
    printf("Error in sys_irqsetpolicy\n");
    return 1;
  }

  // if (sys_irqenable(&hook_id) != OK) {
  //   printf("Error in sys_irqenable\n");
  //   return 1;
  // }

  return OK;
}

int(timer_unsubscribe_int)() {
  // if (sys_irqdisable(&hook_id) != OK) {
  //   printf("Error in sys_disable\n");
  //   return 1;
  // }

  if (sys_irqrmpolicy(&hook_id_timer) != OK) {
    printf("Error in sys_irqrmpolicy\n");
    return 1;
  }

  return OK;
}

void(timer_int_handler)() {
  counter++;
}

int(timer_get_conf)(uint8_t timer, uint8_t *st) {
  // /* To be implemented by the students */

  // read back command
  uint32_t timer_cmd = TIMER_RB_CMD | TIMER_RB_COUNT_ | TIMER_RB_SEL(timer);

  sys_outb(TIMER_CTRL, timer_cmd);

  util_sys_inb(TIMER_0 + timer, st);

  return OK;
}

int(timer_display_conf)(uint8_t timer, uint8_t st, enum timer_status_field field) {
  union timer_status_field_val thing;
  switch (field) {
    case tsf_all: {
      thing.byte = st;
      break;
    }

    case tsf_initial: { // bits 54 Counter Initialization
      //     01 LSB only
      //     10 MSB only
      //     11 LSB followed by MSB

      // extract bits 54 from st
      uint8_t mask = 0x30; // 0b00110000

      // check if bits 54 are 01, 10 or 11
      if ((st & mask) == TIMER_LSB) {
        thing.in_mode = LSB_only;
      }
      else if ((st & mask) == TIMER_MSB) {
        thing.in_mode = MSB_only;
      }
      else if ((st & mask) == TIMER_LSB_MSB) {
        thing.in_mode = MSB_after_LSB;
      }
      else {
        thing.in_mode = INVAL_val;
      }
      break;
    }

    case tsf_mode: { // bits 321 Counting Mode
      //     000 0
      //     001 1
      //     x10 2
      //     x11 3
      //     100 4
      //     101 5

      // extract bits 321 from st
      uint8_t mask = 0x0E; // 0b1110

      // shift bits 321 to the right once
      uint8_t mode_num = (st & mask) >> 1;

      // make the selection of the mode from the bits
      if (mode_num > 5) {
        // subtract 0b0100 from the mode number
        thing.count_mode = mode_num - 4;
      }
      else {
        thing.count_mode = mode_num;
      }
      break;
    }

    case tsf_base: { // check if last bit is true or false
      thing.bcd = (st & 1) == 1;
      break;
    }
  }

  timer_print_config(timer, field, thing);
  return OK;
}

// int(timer_print_config)(uint8_t timer, enum timer_status_field field, union timer_status_field_val val) {
//   switch (field) {
//     case tsf_all: {
//       printf("The config of timer %d is: %x\n", timer, val.byte);
//       break;
//     }

//     case tsf_initial: {
//       switch (val.in_mode) {
//         case LSB_only: {
//           printf("The mode of the timer %d is: LSB only\n", timer);
//           break;
//         }

//         case MSB_only: {
//           printf("The mode of the timer %d is: MSB only\n", timer);
//           break;
//         }

//         case MSB_after_LSB: {
//           printf("The mode of the timer %d is: MSB after LSB\n", timer);
//           break;
//         }

//         case INVAL_val: {
//           printf("The mode of the timer %d is: invalid \n", timer);
//           break;
//         }
//       }
//       break;
//     }

//     case tsf_mode: {
//       printf("The timer is in counting mode %d\n", val.count_mode);
//       break;
//     }

//     case tsf_base: {
//       if (val.bcd) {
//         printf("The timer is in BCD mode\n");
//       }
//       else {
//         printf("The timer is in binary mode\n");
//       }
//       break;
//     }
//   }
//   return OK;
// }
// thought i had to implement it :P
