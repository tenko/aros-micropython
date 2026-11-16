#ifndef MICROPY_INCLUDED_RPI_MINI_UART_H
#define MICROPY_INCLUDED_RPI_MINI_UART_H

#include <stdint.h>
#include <stdbool.h>

void mini_uart_init();
bool mini_uart_txdone(void);
void mini_uart_putc(char c);
uint32_t mini_uart_getc(void);
uint32_t mini_uart_rx_state(void);
bool mini_uart_rx_wait(uint32_t timeout_us);
void mini_uart_set_speed(uint32_t speed);

void isr_irq_mini_uart(void);

#endif // MICROPY_INCLUDED_RPI_MINI_UART_H
