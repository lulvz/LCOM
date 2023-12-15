#include "keyboard.h"

#include <lcom/lcf.h>

#include <stdint.h>

#include "i8042.h"

#define WAIT_KBC 20000
#define TRIES 5

int hook_id_keyboard = 1;  // from 0 to 31
uint8_t data = 0x0;

int keyboard_subscribe_int(uint8_t *bit_no) {
  
  *bit_no = hook_id_keyboard;

  if (sys_irqsetpolicy(KEYBOARD_IRQ, (IRQ_EXCLUSIVE | IRQ_REENABLE), &hook_id_keyboard) != OK) {
    printf("Error in sys_irqsetpolicy\n");
    return 1;
  }

  return OK;
}

int keyboard_unsubscribe_int() {
  if (sys_irqrmpolicy(&hook_id_keyboard) != OK) {
    printf("Error in sys_irqrmpolicy\n");
    return 1;
  }

  return OK;
}

int kbc_read_data() {
  uint8_t stat = 0;
  uint8_t tmp_data;
  int retry_counter = TRIES;

  while (retry_counter > 0) {
    // read status
    if (util_sys_inb(KBC_ST_REG, &stat) != OK) {
      printf("Error in util_sys_inb\n");
      return 1;
    }

    /* check if the 8042 output buffer is empty */
    if (stat & KBC_ST_OBF) {

      util_sys_inb(KBC_OUT_BUF, &tmp_data); /* ass. it returns OK */

      printf("%d\n", stat & (KBC_PAR_ERR | KBC_TO_ERR));
      // check for parity or timout error
      if ((stat & (KBC_PAR_ERR | KBC_TO_ERR)) == 0) {
        data=tmp_data;
        // printf("data: %x\n", data);
        return OK;
      } else {
        printf("Error in kbc_read_data: %d\n", stat);
        return 1;
      }
    }
    retry_counter--;
  }
  return 1;
}

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

void (kbc_ih)() {
  kbc_read_data();
}

int kbd_enable() {
  uint8_t command_byte;

  if(sys_outb(KBC_CMD_REG, READ_CMD_BYTE) != OK) {
    printf("Error in sys_outb\n");
    return 1;
  }

  if(util_sys_inb(KBC_OUT_BUF, &command_byte) != OK) {
    printf("Error in util_sys_inb\n");
    return 1;
  }

  command_byte |= BIT(0);

  if(sys_outb(KBC_CMD_REG, KBC_OUT_BUF) != OK) {
    printf("Error in sys_outb\n");
    return 1;
  }

  if(sys_outb(WRITE_CMD_BYTE, command_byte) != OK) {
    printf("Error in sys_outb\n");
    return 1;
  }
  return OK;
}
