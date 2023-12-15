#include <lcom/lcf.h>
#include <lcom/utils.h>

int(util_get_LSB)(uint16_t val, uint8_t *lsb) {
  /* To be implemented by the students */
  *lsb = (uint8_t)(val & 0xFF);

  return OK;
}

int(util_get_MSB)(uint16_t val, uint8_t *msb) {
  /* To be implemented by the students */
  *msb = (uint8_t) ((val & 0xFF00) >> 8);
  
  return OK;
}

int (util_sys_inb)(int port, uint8_t *value) {
  // returns an 8 bit value instead of a 32 bit one in the value pointer
  /* To be implemented by the students */
  uint32_t st;
  int r = sys_inb(port, &st);

  *value = (uint8_t)st;

  return r;
}
