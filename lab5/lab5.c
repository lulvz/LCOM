// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <lcom/lab5.h>

#include <stdint.h>
#include <stdio.h>

#include <machine/int86.h>

// Any header files included below this line should have been created by you
#include "keyboard.h"
#include "video_gr.h"
#include <lcom/timer.h>

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab5/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab5/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(video_test_init)(uint16_t mode, uint8_t delay) {
  void* video_mem = vg_init(mode);
  if (video_mem == NULL) {
    printf("Error initializing video mode 0x%x with delay %d seconds \n", mode, delay);
    return 1;
  }

  // vg_draw_hline(3, 3, 100, 0xff);
  sleep(delay);
  vg_exit();

  return OK;
}

extern uint8_t data;

int(video_test_rectangle)(uint16_t mode, uint16_t x, uint16_t y,
                          uint16_t width, uint16_t height, uint32_t color) {
  vg_init(mode);

  vg_draw_rectangle(x, y, width, height, color);

  uint8_t bit_no = 0x0;
  keyboard_subscribe_int(&bit_no);
  uint8_t scancodes[2] = {0,0};
  bool readagain = false;

  uint32_t irq_set = BIT(bit_no);
  int ipc_status;
  message msg;

  while (data != 0x81) {
    if (driver_receive(ANY, &msg, &ipc_status) != 0) {
      printf("driver_receive failed with: %d", ipc_status);
      continue;
    }
    if (is_ipc_notify(ipc_status)) {
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:
          if (msg.m_notify.interrupts & irq_set) {
            kbc_ih();
            if (readagain) {
              scancodes[1] = data;
              readagain = false;
            }
            else {
              scancodes[0] = data;
              if (scancodes[0] == 0xE0) {
                readagain = true;
              }
            }
          }
          break;
        default:
          break;
      }
    }
  }

  vg_exit();
  keyboard_unsubscribe_int();

  return OK;
}

int(video_test_pattern)(uint16_t mode, uint8_t no_rectangles, uint32_t first, uint8_t step) {
  uint8_t bit_no = 0x0;
  if(keyboard_subscribe_int(&bit_no) != OK) {
    printf("Error subscribing keyboard interrupts \n");
    return 1;
  }
  uint8_t scancodes[2] = {0,0};
  bool readagain = false;

  uint32_t irq_set = BIT(bit_no);
  int ipc_status;
  message msg;

  vg_init(mode);
  
  vg_draw_pattern(no_rectangles, first, step);

  while (data != 0x81) {
    if (driver_receive(ANY, &msg, &ipc_status) != 0) {
      printf("driver_receive failed with: %d", ipc_status);
      continue;
    }
    if (is_ipc_notify(ipc_status)) {
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:
          if (msg.m_notify.interrupts & irq_set) {
            kbc_ih();
            if (readagain) {
              scancodes[1] = data;
              readagain = false;
            }
            else {
              scancodes[0] = data;
              if (scancodes[0] == 0xE0) {
                readagain = true;
              }
            }
          }
          break;
        default:
          break;
      }
    }
  }

  vg_exit();
  keyboard_unsubscribe_int();

  return OK;
}

int(video_test_xpm)(xpm_map_t xpm, uint16_t x, uint16_t y) {
  uint8_t bit_no = 0x0;
  if(keyboard_subscribe_int(&bit_no) != OK) {
    printf("Error subscribing keyboard interrupts \n");
    return 1;
  }
  uint8_t scancodes[2] = {0,0};
  bool readagain = false;

  // initialize in indexed mode
  vg_init(0x105);
  xpm_image_t img;
  uint8_t* map = xpm_load(xpm, XPM_INDEXED, &img);
  vg_draw_xpmmap(x, y, map, &img);

  uint32_t irq_set = BIT(bit_no);
  int ipc_status;
  message msg;

  while (data != 0x81) {
    if (driver_receive(ANY, &msg, &ipc_status) != 0) {
      printf("driver_receive failed with: %d", ipc_status);
      continue;
    }
    if (is_ipc_notify(ipc_status)) {
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:
          if (msg.m_notify.interrupts & irq_set) {
            kbc_ih();
            if (readagain) {
              scancodes[1] = data;
              readagain = false;
            }
            else {
              scancodes[0] = data;
              if (scancodes[0] == 0xE0) {
                readagain = true;
              }
            }
          }
          break;
        default:
          break;
      }
    }
  }

  vg_exit();
  keyboard_unsubscribe_int();

  return OK;
}

extern long counter;
int(video_test_move)(xpm_map_t xpm, uint16_t xi, uint16_t yi, uint16_t xf, uint16_t yf,
                     int16_t speed, uint8_t fr_rate) {
  uint8_t bit_no_kb = 0x0;
  if(keyboard_subscribe_int(&bit_no_kb) != OK) {
    printf("Error subscribing keyboard interrupts \n");
    return 1;
  }
  uint8_t bit_no_timer = 0x0;
  if(timer_subscribe_int(&bit_no_timer) != OK) {
    printf("Error subscribing timer interrupts \n");
    return 1;
  }
  uint8_t scancodes[2] = {0,0};
  bool readagain = false;

  vg_init(0x105);
  xpm_image_t img;
  uint8_t* map = xpm_load(xpm, XPM_INDEXED, &img);

  uint16_t interruptFrames = sys_hz()/fr_rate;
  
  uint32_t irq_set_kb = BIT(bit_no_kb);
  uint32_t irq_set_timer = BIT(bit_no_timer);
  int ipc_status;
  message msg;
  int frame_counter=0;
  while (data != 0x81) {
    if (driver_receive(ANY, &msg, &ipc_status) != 0) {
      printf("driver_receive failed with: %d", ipc_status);
      continue;
    }
    if (is_ipc_notify(ipc_status)) {
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:
          if (msg.m_notify.interrupts & irq_set_kb) {
            kbc_ih();
            if (readagain) {
              scancodes[1] = data;
              readagain = false;
            }
            else {
              scancodes[0] = data;
              if (scancodes[0] == 0xE0) {
                readagain = true;
              }
            }
          }
          if (msg.m_notify.interrupts & irq_set_timer) {
            timer_int_handler();
            if (counter % interruptFrames == 0) {
              frame_counter++;
              // If speed is positive it should be understood as the displacement in pixels between consecutive frames.
              if(speed > 0) {
                vg_erase_xpmmap(xi, yi, map, &img);
                if(xi == xf) {
                  // if new y is greater than final y, set new y to final y
                  if(yi + speed > yf)
                    yi = yf;
                  else
                    yi += speed;
                } else if (yi == yf) {
                  if(xi + speed > xf)
                    xi = xf;
                  else
                    xi += speed;
                }
                vg_draw_xpmmap(xi, yi, map, &img);
              } 
              // If the speed is negative it should be understood as the number of frames required for a displacement of one pixel.
              else {
                frame_counter++;
                // each abs(speed) frames, move one pixel
                if(frame_counter == abs(speed)) {
                  vg_erase_xpmmap(xi, yi, map, &img);
                  if(xi == xf) {
                    // if new y is greater than final y, set new y to final y
                    if(yi + 1 > yf)
                      yi = yf;
                    else
                      yi += 1;
                  } else if (yi == yf) {
                    if(xi + 1 > xf)
                      xi = xf;
                    else
                      xi += 1;
                  }
                  vg_draw_xpmmap(xi, yi, map, &img);
                  frame_counter = 0;
                }
              }
            }
          }
          break;
        default:
          break;
      }
    }
  }

  vg_exit();
  keyboard_unsubscribe_int();
  timer_unsubscribe_int();

  return OK;
}
