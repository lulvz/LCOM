#include "mouse.h"
#include "lcom/lab4.h"
#include "../lab3/i8042.h"

#define MOUSE_TRIES 5

int hook_id_mouse = 2;

uint8_t scancode;
uint8_t bytes[3];

uint8_t byte_counter = 0;

int(mouse_subscribe_int(uint8_t *bit_no)) {
  *bit_no = hook_id_mouse;

  if(sys_irqsetpolicy(KBD_AUX_IRQ, (IRQ_REENABLE | IRQ_EXCLUSIVE), &hook_id_mouse)) {
    printf("Error setting policy\n");
    return 1;
  }

  return OK; 
}

int(mouse_unsubscribe_int()) {
  if(sys_irqrmpolicy(&hook_id_mouse) != OK) {
    printf("Error unsubscribing interrupts\n");
    return 1;
  }

  return OK;
}

bool(check_output_buffer_full()) {
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
  // bruh
}

bool(check_input_buffer()) {
  // uint8_t stat;
  return true;
}


void (mouse_ih)() {
  int tries = MOUSE_TRIES;
  // while ckeck output buffer try to receive using util_sys_inb from the OUT_BUF
  while(tries > 0) {
    if (check_output_buffer_full()) {
      if (util_sys_inb(KBC_OUT_BUF, &scancode) != OK) {
        scancode = 0x0;
        printf("Error in util_sys_inb in mouse\n");
        return;
      }
    }
    tries--;
  }
}

void build_packet(struct packet * p) {
  p->bytes[0] = bytes[0];
  p->bytes[1] = bytes[1];
  p->bytes[2] = bytes[2];

  p->lb = bytes[0] & LEFT_BUTTON;
  p->rb = bytes[0] & RIGHT_BUTTON;
  p->mb = bytes[0] & MIDDLE_BUTTON;

  if (bytes[0] & MSB_X_DELTA) {
    p->delta_x = bytes[1] - 256;
  } else {
    p->delta_x = bytes[1];
  }

  if (bytes[0] & MSB_Y_DELTA) {
    p->delta_y = bytes[2] - 256;
  } else {
    p->delta_y = bytes[2];
  }

  p->x_ov = bytes[0] & X_OVERFLOW;
  p->y_ov = bytes[0] & Y_OVERFLOW;
}

bool (mouse_stream_handler()) {
  if(byte_counter == 0) {
    mouse_ih();
    bytes[0] = scancode;
    byte_counter++;
    return false;
  } else if (byte_counter == 1) {
    mouse_ih();
    bytes[1] = scancode; 
    byte_counter++;
    return false;
  } else if (byte_counter == 2) {
    mouse_ih();
    bytes[2] = scancode;
    byte_counter = 0;
    struct packet p;
    build_packet(&p);
    mouse_print_packet(&p);
    return true;
  }
  return false;
}
