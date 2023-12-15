#ifndef VIDEO_GR_H
#define VIDEO_GR_H
#include <lcom/lcf.h>
#include "alphabet.h"

#include <stdint.h>

struct sprite {
  char *name;
  xpm_image_t img; // info about the pixmap
  uint8_t* pixmap; // the pixmap 

  // need to allow negative values
  uint16_t x, y;
  int sx, sy; // speed on x and y axis  
};

struct animated_sprite {
  char *name;
  struct sprite *sprites;
  int aspeed; // animation speed
  int curr_aspeed; // frames till next sprite
  int num_sprites; // number of sprites in the animation
  int curr_sprite; // current sprite being displayed
};

struct sprite *create_sprite(char * name, xpm_map_t pic, uint16_t x, uint16_t y, uint16_t sx, uint16_t sy);

struct animated_sprite *create_animated_sprite(char * name, xpm_map_t pic[], int num_sprites, uint16_t x, uint16_t y, int aspeed);

unsigned get_bytes_per_pixel();
unsigned get_v_res();
unsigned get_h_res();

void destroy_sprite(struct sprite *sp);

void destroy_animated_sprite(struct animated_sprite *sp);

void* (vg_init)(uint16_t mode);
int (vg_exit_custom)();

// copies temporary buffer to video_mem
void (copy_mem)();

int (vg_draw_pixel)(uint16_t x, uint16_t y, uint32_t color);

int (vg_draw_char)(uint16_t x, uint16_t y, char ch, uint32_t color);
int (vg_draw_string)(uint16_t x, uint16_t y, char *str, uint32_t color);

int (vg_draw_hline)(uint16_t x, uint16_t y, uint16_t len, uint32_t color);
int (vg_draw_rectangle)(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color);

uint32_t get_color(uint32_t first, uint8_t step, uint16_t row, uint16_t col);
int (vg_draw_pattern)(uint8_t no_rectangles, uint32_t first, uint8_t step);

int (vg_draw_xpmmap)(uint16_t x, uint16_t y, uint8_t* xpmmap, xpm_image_t *img);
int (vg_erase_xpmmap)(uint16_t x, uint16_t y, uint8_t* xpmmap, xpm_image_t *img);

int (vg_draw_sprite)(struct sprite *sp);
int (vg_draw_animated_sprite)(struct animated_sprite *sp);
int (vg_erase_sprite)(struct sprite *sp);

#endif
