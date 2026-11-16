#include <stdbool.h>
#include "bcm283x_gpio.h"
#include "bcm283x_aux.h"
#include "rpi.h"
#include "mini_uart.h"
#include "py/mphal.h"
#include "py/runtime.h"

mini_uart_t volatile *mini_uart = 0;

#define RX_CH        (mini_uart->IO)
#define TX_FULL      ((mini_uart->LSR & 0x20) == 0x0)
#define TX_CH(c)     (mini_uart->IO = (c))
#define RX_EMPTY       !(mini_uart->LSR & 1U)
#define RX_READ(c)     (c = RX_CH)

void isr_irq_mini_uart(void) {
    return;
}

void mini_uart_set_speed(uint32_t speed) {
    mini_uart->CNTL = 0;   // disable mini uart
    mini_uart->BAUD = ((rpi_freq_core() / speed) >> 3) - 1;
    mini_uart->CNTL = 3;   // enable mini uart
}

void mini_uart_init() {
    mini_uart = (mini_uart_t*) AUX_MU;

    // set GPIO14, GPIO15, alternate function 5
    IOREG(GPFSEL1) = (GPF_ALT_5 << (3*4)) | (GPF_ALT_5 << (3*5));

    // UART basic settings

    AUX_ENABLES |= AUX_FLAG_MU;
    mini_uart->IER = 0;    // disable interrupts
    mini_uart->CNTL = 0;   // disable mini uart
    mini_uart->IER = 0;    // disable receive/transmit interrupts12
    mini_uart->IIR = 0xC6; // enable FIFO(0xC0), clear FIFO(0x06)
    mini_uart->MCR = 0;    // set RTS to High

    // data and speed (mini uart is always parity none, 1 start bit 1 stop bit)
    mini_uart->LCR = 3;    // 8 bits; Note: BCM2835 manual p14 is incorrect
    mini_uart->BAUD = ((rpi_freq_core() / 115200) >> 3) - 1;
    //    mini_uart->BAUD = 270; // 1115200 bps @ core_freq=250
    //    mini_uart->BAUD = 434; // 1115200 bps @ core_freq=400

    // enable transmit and receive
    mini_uart->CNTL = 3;
};

bool mini_uart_txdone(void) {
    return TX_FULL == 0;
}

void mini_uart_putc(char c) {
    while(TX_FULL) {
    }
    TX_CH(c);
}

uint32_t mini_uart_getc(void) {
    uint32_t c;

    while(RX_EMPTY) {
    }
    RX_READ(c);
    return c & 0xffU;
}

uint32_t mini_uart_rx_state(void) {
    return !RX_EMPTY;
}

// Waits at most timeout microseconds for at least 1 char to become ready for reading.
// Returns true if something available, false if not.
bool mini_uart_rx_wait(uint32_t timeout_us) {
    int i = 0;
    for (;;) {
        if (mini_uart_rx_state() != 0) {
            return true; // have at least 1 char ready for reading
        }
        if (i > 9) {
            return false;
        }
        mp_hal_delay_us(timeout_us / 10);
        mp_event_handle_nowait();
        i += 1;
    }
}
