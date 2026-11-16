#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "py/mphal.h"
#include "py/runtime.h"

#include "rpi.h"

uint32_t mp_hal_disable(void) {
    aros_disable();
    return 0;
}

void mp_hal_enable(uint32_t i) {
    aros_enable();
}

// Time and delay

void mp_hal_delay_ms(mp_uint_t ms) {
    uint64_t end_time;

    end_time = rpi_systime() + ms * 1000;
    while(rpi_systime() < end_time) {
        mp_handle_pending(false);
    }

    return;
}

void mp_hal_delay_us(mp_uint_t us) {
    uint64_t end_time;

    end_time = rpi_systime() + us;
    while(rpi_systime() < end_time);

    return;
}

mp_uint_t mp_hal_get_cpu_freq(void) {
    return rpi_freq_cpu();
}

mp_uint_t mp_hal_ticks_cpu(void) {
    return rpi_systime();
}

mp_uint_t mp_hal_ticks_us(void) {
    return rpi_systime();
}

mp_uint_t mp_hal_ticks_ms(void) {
    return rpi_systime() / 1000;
}

void mp_hal_get_random(size_t n, uint8_t *buf) {
    for (size_t i = 0; i < n; ++i) {
        buf[i] = random() & 0xFF;
    }
}
