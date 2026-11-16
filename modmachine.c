#include <stdio.h>
#include "modmachine.h"

#define asm __asm__

static void mp_machine_idle(void) {
    asm ("wfi");
    mp_event_handle_nowait(); // handle any events after possibly a long wait (eg feed WDT)
}

#define MICROPY_PY_MACHINE_EXTRA_GLOBALS \
    { MP_ROM_QSTR(MP_QSTR_Pin), MP_ROM_PTR(&machine_pin_type) }, \
    { MP_ROM_QSTR(MP_QSTR_I2C), MP_ROM_PTR(&machine_i2c_type) }, \
    { MP_ROM_QSTR(MP_QSTR_SPI), MP_ROM_PTR(&machine_spi_type) }, \
    { MP_ROM_QSTR(MP_QSTR_PWM), MP_ROM_PTR(&machine_pwm_type) }, \
