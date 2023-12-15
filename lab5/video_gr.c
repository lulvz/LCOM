#include "video_gr.h"

static vbe_mode_info_t info;
static void* video_mem;
static unsigned h_res;
static unsigned v_res;
static unsigned bits_per_pixel;
static bool indexed_mode;

unsigned get_bytes_per_pixel() {
  // good aproximation to get the bytes per pixel of each mode
  return (bits_per_pixel + 7) / 8; 
}

void* (vg_init)(uint16_t mode) {
  indexed_mode = mode==0x105;
  vbe_get_mode_info(mode, &info);

  reg86_t r86;
  memset(&r86, 0, sizeof(r86));

  r86.intno = 0x10;
  r86.ah = 0x4F;
  r86.al = 0x02;
  r86.bx = 1<<14 | (mode);
  
  // Bios call
  if( sys_int86(&r86) != OK ) {
    printf("vg_init: sys_int86() failed \n");
    return NULL;
  }

  // Set global variables
  h_res = info.XResolution;
  v_res = info.YResolution;
  bits_per_pixel = info.BitsPerPixel;

  unsigned int vram_base = (phys_bytes) info.PhysBasePtr;
  unsigned int vram_size = h_res * v_res * get_bytes_per_pixel(); 

  struct minix_mem_range mr;
  mr.mr_base = (phys_bytes) vram_base;	
  mr.mr_limit = mr.mr_base + vram_size;

  // Request permission to map memory
  int r;
  if( OK != (r = sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr)))
    panic("sys_privctl (ADD_MEM) failed: %d\n", r);

  // Map memory
  video_mem = vm_map_phys(SELF, (void *)mr.mr_base, vram_size);
  if(video_mem == MAP_FAILED)
    panic("couldn't map video memory");

  return video_mem;
}

int (vg_draw_pixel)(uint16_t x, uint16_t y, uint32_t color) {
  if (x > h_res || y > v_res)
    return 1;
  uint8_t *ptr;

  ptr = (uint8_t*)video_mem + (y * h_res + x) * get_bytes_per_pixel(); 

  for (uint8_t i = 0; i < get_bytes_per_pixel(); i++) {
    *ptr = color;
    color >>= 8;
    ptr++;
  }

  return 0;
}

int (vg_draw_hline)(uint16_t x, uint16_t y, uint16_t len, uint32_t color) {
  if (x + len > h_res || y > v_res)
    return 1;

  for (int i = 0; i < len; i++) {
    vg_draw_pixel(x + i, y, color); 
  }

  return 0;
}

int (vg_draw_rectangle)(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color) {
  if (x + width > h_res || y + height > v_res)
    return 1;

  for (int i = 0; i < height; i++) {
    vg_draw_hline(x, y + i, width, color);
  }

  return 0;
}

uint32_t get_color(uint32_t first, uint8_t step, uint16_t row, uint16_t col) {
  uint32_t color = 0;
  uint8_t red = (first >> 16) & 0xFF;
  uint8_t green = (first >> 8) & 0xFF;
  uint8_t blue = first & 0xFF;

  red = (red + col * step) % (1<<info.RedMaskSize);
  green = (green + row * step) % (1<<info.GreenMaskSize);
  blue = (blue + (col + row) * step) % (1<<info.BlueMaskSize);

  color = (red << 16) | (green << 8) | blue;

  return color;
}

int (vg_draw_pattern)(uint8_t no_rectangles, uint32_t first, uint8_t step) {
  unsigned rect_width = h_res / no_rectangles;
  unsigned rect_height = v_res / no_rectangles;
  unsigned rect_remainder_width = h_res % no_rectangles;
  unsigned rect_remainder_height = v_res % no_rectangles;

  uint32_t color = first;

  uint16_t i, j;
  for(i = 0; i < no_rectangles; i++) { // row
    for(j = 0; j < no_rectangles; j++) { // col
      if(indexed_mode) {
        color = (first + (i * no_rectangles + j) * step) % (1 << bits_per_pixel);
      } else {
        color = get_color(first, step, i, j);
      }
      vg_draw_rectangle(j * rect_width, i * rect_height, rect_width, rect_height, color);
    }
  }
  // fill last remainder column with black
  vg_draw_rectangle(j*rect_width, 0, rect_remainder_width, v_res, 0);

  // fill last row remainder with black
  vg_draw_rectangle(0, i*rect_height, h_res, rect_remainder_height, 0);

  return 0;
}

int (vg_draw_xpmmap)(uint16_t x, uint16_t y, uint8_t* xpmmap, xpm_image_t *img) {
  uint32_t color;
  for(size_t i = 0; i<img->height; i++) {
    for(size_t j = 0; j<img->width; j++) {
        color = 0;
        // get the color of the pixel depending on the number of bytes per pixel
        for(size_t byte = 0; byte < get_bytes_per_pixel(); byte++)
          // get the color byte by byte and shift it to the correct position
          color |= (xpmmap[(j + i * img->width) * get_bytes_per_pixel() + byte]) << (byte * 8);
        // only draw the pixel if it's not the transparency color
        if(color != xpm_transparency_color(img->type))
          if(vg_draw_pixel(x+j, y+i, color) != OK)
            return 1;
    }
  }
  return 0;
}

// would work for any mode, indexed or direct
int (vg_erase_xpmmap)(uint16_t x, uint16_t y, uint8_t* xpmmap, xpm_image_t *img) {
  for(size_t i = 0; i<img->height; i++) {
    for(size_t j = 0; j<img->width; j++) {
      // get the transparency color for the type of the image
      if(vg_draw_pixel(x+j, y+i, xpm_transparency_color(img->type)) != OK)
        return 1;
    }
  }
  return 0;
}
