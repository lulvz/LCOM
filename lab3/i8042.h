#ifndef I8042_H
#define I8042_H

// #define BIT(n) (0x01<<(n))

#define ESC_MAKE         0x01
#define ESC_BREAK        0x81

#define BREAK BIT        (7)

#define KBC_ST_REG 		   0x64
#define KBC_CMD_REG      0x64
#define KBC_IN_BUF		   0x64
#define KBC_OUT_BUF 		 0x60
#define KBC_ST_IBF       0x01 // input buffer full
#define KBC_ST_OBF       BIT(0) // output buffer full

#define READ_CMD_BYTE    0x20
#define WRITE_CMD_BYTE   0x60

#define KBC_PAR_ERR BIT  (7)
#define KBC_TO_ERR  BIT  (6)

#define SCANCODE_DOUBLE  0xE0

// mouse
#define LEFT_BUTTON BIT (0)
#define RIGHT_BUTTON BIT(1)
#define MIDDLE_BUTTON BIT(2)
#define MSB_X_DELTA BIT (4)
#define MSB_Y_DELTA BIT (5)
#define X_OVERFLOW BIT  (6)
#define Y_OVERFLOW BIT  (7)

#endif
