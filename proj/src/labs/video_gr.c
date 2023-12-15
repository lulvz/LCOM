#include "video_gr.h"

/// @brief Struct containing the information of a given mode.
static vbe_mode_info_t info;

/// @brief Pointer to the video memory address.
static void* video_mem;

/// @brief Pointer to the video buffer address.
static void* video_buffer;

/// @brief Horizontal resolution of the screen.
static unsigned h_res;

/// @brief Vertical resolution of the screen.
static unsigned v_res;

/// @brief Bits per pixel of the current mode.
static unsigned bits_per_pixel;

/// @brief Flag that indicates if the current mode is indexed.
static bool indexed_mode;

/// @brief Calculates the number of bytes per pixel of the current mode, given the bits per pixel.
/// A simple approximation is used, since the number of bits per pixel is not always a multiple of 8.
/// @return Returns the number of bytes per pixel of the current mode.
unsigned get_bytes_per_pixel() {
  // good aproximation to get the bytes per pixel of each mode
  return (bits_per_pixel + 7) / 8;
}

/// @brief Getter for the horizontal resolution of the screen.
/// @return Returns the horizontal resolution of the screen.
unsigned get_h_res() {
  return h_res;
}

/// @brief Getter for the vertical resolution of the screen.
/// @return Returns the vertical resolution of the screen.
unsigned get_v_res() {
  return v_res;
}

/// @brief Builds a sprite from a given xpm.
/// The image is loaded in 32 bit color mode.
/// @param name Name of the sprite.
/// @param pic Xpm of the sprite.
/// @param x Initial x position of the sprite.
/// @param y Initial y position of the sprite.
/// @param sx Horizontal speed of the sprite.
/// @param sy Vertical speed of the sprite.
/// @return Returns a pointer to the sprite created.
struct sprite * create_sprite(char * name, xpm_map_t pic, uint16_t x, uint16_t y, uint16_t sx, uint16_t sy) {
  struct sprite *sp = (struct sprite*) malloc(sizeof(struct sprite));
  if (sp == NULL)
    return NULL;

  sp->name = name;
  if (xpm_load(pic, XPM_8_8_8_8, &sp->img) == NULL) {
    free(sp);
    return NULL;
  }

  sp->pixmap = sp->img.bytes;
  sp->x = x;
  sp->y = y;
  sp->sx = sx;
  sp->sy = sy;

  return sp;
}

struct animated_sprite *create_animated_sprite(char * name, xpm_map_t pic[], int num_sprites, uint16_t x, uint16_t y, int aspeed) {
  struct animated_sprite *sp = malloc(sizeof(struct animated_sprite));
  if (sp == NULL)
    return NULL;

  sp->name = name;
  sp->sprites = malloc(num_sprites * sizeof(struct sprite));
  if (sp->sprites == NULL) {
    free(sp);
    return NULL;
  }

  sp->num_sprites = num_sprites;
  sp->aspeed = aspeed;
  sp->curr_aspeed = aspeed;
  sp->curr_sprite = 0;

  for(int i = 0; i < num_sprites; i++) {
    if (xpm_load(pic[i], XPM_8_8_8_8, &sp->sprites[i].img) == NULL) {
      free(sp->sprites);
      free(sp);
      return NULL;
    }

    sp->sprites[i].pixmap = sp->sprites[i].img.bytes;
    sp->sprites[i].x = x;
    sp->sprites[i].y = y;
    sp->sprites[i].sx = 0;
    sp->sprites[i].sy = 0;
  }

  return sp;
}

/// @brief Destroys a given sprite.
/// @param sp Sprite to be destroyed.
void destroy_sprite(struct sprite *sp) {
  if (sp == NULL)
    return;

  if (sp->pixmap)
    free(sp->pixmap);
  free(sp);
}

void destroy_animated_sprite(struct animated_sprite *sp) {
  if (sp == NULL)
    return;

  if (sp->sprites) {
    for(int i = 0; i < sp->num_sprites; i++) {
      if (sp->sprites[i].pixmap)
        free(sp->sprites[i].pixmap);
    }
    free(sp->sprites);
  }
  free(sp);
}

// todo implement page flipping (less important)

/// @brief Initializes the video module in the specified mode.
/// @param mode Mode to be initialized.
/// @return Returns a pointer to the video memory address.
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
  unsigned int vram_size = h_res * v_res * ((bits_per_pixel + 7) / 8);

  video_buffer = malloc(vram_size);

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

/// @brief Returns to default Minix 3 text mode.
/// @return Returns 0 upon success and non-zero otherwise.
int vg_exit_custom() {
  free(video_buffer);

  reg86_t r86;
  memset(&r86, 0, sizeof(r86));

  r86.intno = 0x10;
  r86.ah = 0x00;
  r86.al = 0x03;
  
  // Bios call
  if( sys_int86(&r86) != OK ) {
    printf("vg_exit: sys_int86() failed \n");
    return 1;
  }

  return OK;
}

/// @brief Copies the contents of the video buffer to the video memory.
void copy_mem() {
  memcpy(video_mem, video_buffer, h_res * v_res * get_bytes_per_pixel());
}

/// @brief Draws a pixel in the specified coordinates with the specified color.
/// @param x X coordinate of the pixel.
/// @param y Y coordinate of the pixel.
/// @param color Color of the pixel.
/// @return Returns 0 upon success and non-zero otherwise.
int (vg_draw_pixel)(uint16_t x, uint16_t y, uint32_t color) {

  if ((x > (h_res-1)) || (y > (v_res-1))) {
    // printf("vg_draw_pixel: invalid coordinates (%d, %d)\n", x, y);
    return 1;
  }
  uint8_t *ptr;

  ptr = (uint8_t*)video_buffer + (y * h_res + x) * ((bits_per_pixel + 7) / 8);

  for (uint8_t i = 0; i < get_bytes_per_pixel(); i++) {
    *ptr = color;
    color >>= 8;
    ptr++;
  }

  return 0;
}

/// @brief Draws a character in the specified coordinates with the specified color.
/// @param x X coordinate of the character.
/// @param y Y coordinate of the character.
/// @param ch Character to be drawn.
/// @param color Color of the character.
/// @return Returns 0 upon success and non-zero otherwise.
int (vg_draw_char)(uint16_t x, uint16_t y, char ch, uint32_t color) {
  if ((x > (h_res-8)) || (y > (v_res-8))) {
    return 1;
  }

  int idx = ch - 32; // get the index of the char in the alphabet array alphabet[95][13]

  // make the char have a bigger size by extrapolation
  for(int i = 0; i < 13; i++) {
    for(int j = 0; j < 8; j++) {
      if(alphabet[idx][i] & (1 << j)) {
        vg_draw_pixel(x + (7-j)*2, y + (12-i)*2, color);
        vg_draw_pixel(x + (7-j)*2 + 1, y + (12-i)*2, color);
        vg_draw_pixel(x + (7-j)*2, y + (12-i)*2 + 1, color);
        vg_draw_pixel(x + (7-j)*2 + 1, y + (12-i)*2 + 1, color);
      }
    }
  }

  return 0;
}

/// @brief Draws a string in the specified coordinates with the specified color.
/// @param x X coordinate of the string.
/// @param y Y coordinate of the string.
/// @param str String to be drawn.
/// @param color Color of the string.
/// @return Returns 0 upon success and non-zero otherwise.
int (vg_draw_string)(uint16_t x, uint16_t y, char *str, uint32_t color) {
  if ((x > (h_res-8)) || (y > (v_res-8))) {
    // printf("vg_draw_string: invalid coordinates (%d, %d)\n", x, y);
    return 1;
  }

  int i = 0;
  while(str[i] != '\0') {
    vg_draw_char(x + (i * 16), y, str[i], color);
    i++;
  }

  return 0;
}

/// @brief Draws a horizontal line in the specified coordinates with the specified color.
/// @param x X coordinate of the line.
/// @param y Y coordinate of the line.
/// @param len Length of the line.
/// @param color Color of the line.
/// @return Returns 0 upon success and non-zero otherwise.
int (vg_draw_hline)(uint16_t x, uint16_t y, uint16_t len, uint32_t color) {
  if ((x + len > h_res) || (y > v_res))
    return 1;

  for (int i = 0; i < len; i++) {
    vg_draw_pixel(x + i, y, color); 
  }

  return 0;
}

/// @brief Draws a xpm image in the specified coordinates.
/// @param x X coordinate of the image.
/// @param y Y coordinate of the image.
/// @param xpmmap XPM map of the image.
/// @param img Image to be drawn.
/// @return Returns 0 upon success and non-zero otherwise.
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
          vg_draw_pixel(x+j, y+i, color);
    }
  }
  return 0;
}

/// @brief Erases a xpm image in the specified coordinates.
/// @param x X coordinate of the image.
/// @param y Y coordinate of the image.
/// @param xpmmap XPM map of the image.
/// @param img Image to be erased.
/// @return Returns 0 upon success and non-zero otherwise.
int (vg_erase_xpmmap)(uint16_t x, uint16_t y, uint8_t* xpmmap, xpm_image_t *img) {
  for(size_t i = 0; i<img->height; i++) {
    for(size_t j = 0; j<img->width; j++) {
      // get the transparency color for the type of the image
      vg_draw_pixel(x+j, y+i, xpm_transparency_color(img->type));
    }
  }
  return 0;
}

/// @brief Draws a sprite specified in the argument.
/// @param sp Sprite to be drawn.
/// @return Returns 0 upon success and non-zero otherwise.
int (vg_draw_sprite)(struct sprite *sp) {
  uint32_t color;
  for(size_t i = 0; i<sp->img.height; i++) {
    for(size_t j = 0; j<sp->img.width; j++) {
        color = 0;
        // get the color of the pixel depending on the number of bytes per pixel
        for(size_t byte = 0; byte < get_bytes_per_pixel(); byte++)
          // get the color byte by byte and shift it to the correct position
          color |= (sp->pixmap[(j + i * sp->img.width) * get_bytes_per_pixel() + byte]) << (byte * 8);
        // only draw the pixel if it's not the transparency color
        if(color != xpm_transparency_color(sp->img.type))
          vg_draw_pixel(sp->x+j, sp->y+i, color);
    }
  }
  return 0;
}

int (vg_draw_animated_sprite)(struct animated_sprite *sp) {
  if (sp->curr_aspeed == 0) {
    sp->curr_sprite = (sp->curr_sprite + 1) % sp->num_sprites;
    sp->curr_aspeed = sp->aspeed;
  } else {
    sp->curr_aspeed--;
  }

  return vg_draw_sprite(&sp->sprites[sp->curr_sprite]);
}

/// @brief Erases a sprite specified in the argument.
/// @param sp Sprite to be erased.
/// @return Returns 0 upon success and non-zero otherwise.
int (vg_erase_sprite)(struct sprite *sp) {
  for(size_t i = 0; i<sp->img.height; i++) {
    for(size_t j = 0; j<sp->img.width; j++) {
      // get the transparency color for the type of the image
      if(sp->pixmap[(j + i * sp->img.width) * get_bytes_per_pixel()] != xpm_transparency_color(sp->img.type))
      vg_draw_pixel(sp->x+j, sp->y+i, xpm_transparency_color(sp->img.type));
    }
  }
  return 0;
}
