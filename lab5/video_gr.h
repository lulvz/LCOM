#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <lcom/lcf.h>
// #include <lcom/xpm.h>
// #include <lcom/vbe.h>

#include <stdint.h>

void* (vg_init)(uint16_t mode);
void* (vbe_map_init)(struct minix_mem_range mr, unsigned int vram_base, unsigned int vram_size);

int (vg_draw_pixel)(uint16_t x, uint16_t y, uint32_t color);
int (vg_draw_hline)(uint16_t x, uint16_t y, uint16_t len, uint32_t color);
int (vg_draw_rectangle)(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color);

uint32_t get_color(uint32_t first, uint8_t step, uint16_t row, uint16_t col);
int (vg_draw_pattern)(uint8_t no_rectangles, uint32_t first, uint8_t step);

int (vg_draw_xpmmap)(uint16_t x, uint16_t y, uint8_t* xpmmap, xpm_image_t *img);
int (vg_erase_xpmmap)(uint16_t x, uint16_t y, uint8_t* xpmmap, xpm_image_t *img);
#endif
