#ifndef ALPHABET_H
#define ALPHABET_H

#include <stdint.h>

// matrix with the alphabet represented as a pixmap (bytes stacked vertically)
// the characters start at DEC(32) = 0x20 = ' ' (space)
// https://stackoverflow.com/a/23130671
uint8_t alphabet[95][13];
#endif
