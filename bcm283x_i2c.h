#ifndef MICROPY_INCLUDED_RPI_BCM283X_I2C_H
#define MICROPY_INCLUDED_RPI_BCM283X_I2C_H

#include "bcm283x.h"
#define BSC0 (0x205000 + IO_BASE)
#define BSC1 (0x804000 + IO_BASE)
#define BSC2 (0x805000 + IO_BASE)

typedef volatile struct _i2c_t {
    uint32_t C;
    uint32_t S;
    uint32_t DLEN;
    uint32_t A;
    uint32_t FIFO;
    uint32_t DIV;
    uint32_t DEL;
    uint32_t CLKT;
} i2c_t;

#define C_I2CEN (1<<15)
#define C_INTR  (1<<10)
#define C_INTT  (1<<9)
#define C_INTD  (1<<8)
#define C_ST    (1<<7)
#define C_CLEAR (3<<4)
#define C_READ  (1)

#define S_CLKT  (1<<9)
#define S_ERR   (1<<8)
#define S_RXF   (1<<7)
#define S_TXE   (1<<6)
#define S_RXD   (1<<5)
#define S_TXD   (1<<4)
#define S_RXR   (1<<3)
#define S_TXW   (1<<2)
#define S_DONE  (1<<1)
#define S_TA    (1)

#endif // MICROPY_INCLUDED_RPI_BCM283X_I2C_H
