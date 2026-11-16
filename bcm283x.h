#ifndef MICROPY_INCLUDED_RPI_BCM283X_H
#define MICROPY_INCLUDED_RPI_BCM283X_H

#include <stdint.h>
#define IOREG(X)  (*(volatile uint32_t *) (X))
#define IO_BASE periiobase
extern uint32_t periiobase;

#endif // MICROPY_INCLUDED_RPI_BCM283X_H
