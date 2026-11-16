#ifndef MICROPY_INCLUDED_RPI_RPI_H
#define MICROPY_INCLUDED_RPI_RPI_H
#include <stdint.h>

void rpi_init();
uint32_t rpi_freq_core();
uint32_t rpi_freq_cpu();
void aros_disable(void);
void aros_enable(void);
volatile uint64_t rpi_systime(void);
volatile uint64_t rpi_elapsed_from(uint64_t t);
void rpi_deinit();

#endif // MICROPY_INCLUDED_RPI_RPI_H
