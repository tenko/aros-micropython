#include "py/objint.h"
#include "rpi.h"
#include "bcm283x_gpio.h"
#include "modmachine.h"

static inline void mp_hal_set_interrupt_char(char c) {}

uint32_t mp_hal_disable(void);
void mp_hal_enable(uint32_t i);

#define mp_hal_pin_od_low(pin) gpio_set_level(((machine_pin_obj_t *)pin)->id, 0)
#define mp_hal_pin_od_high(pin) gpio_set_level(((machine_pin_obj_t *)pin)->id, 1)

void mp_hal_delay_ms(mp_uint_t ms);
void mp_hal_delay_us(mp_uint_t us);
mp_uint_t mp_hal_get_cpu_freq(void);
mp_uint_t mp_hal_ticks_cpu(void);
mp_uint_t mp_hal_ticks_us(void);
mp_uint_t mp_hal_ticks_ms(void);

#define mp_hal_delay_us_fast(us) mp_hal_delay_us(us)
#define mp_hal_quiet_timing_enter() mp_hal_disable()
#define mp_hal_quiet_timing_exit(irq_state) mp_hal_enable(irq_state)

void mp_hal_get_random(size_t n, uint8_t *buf);

#define RAISE_ERRNO(err_flag, error_val) \
    { if (err_flag == -1) \
      { mp_raise_OSError(error_val); } }

// Don't use the unix version of this macro.
#undef MICROPY_INTERNAL_WFE

#if MICROPY_ENABLE_SCHEDULER
// Use minimum 1mSec sleep to make sure there is effectively a wait period:
// something like usleep(500) truncates and ends up calling Sleep(0).
#define MICROPY_INTERNAL_WFE(TIMEOUT_MS) mp_hal_delay_us(1000*MAX(1, (TIMEOUT_MS)))
#else
#define MICROPY_INTERNAL_WFE(TIMEOUT_MS) /* No-op */
#endif
