#ifndef MICROPY_INCLUDED_RPI_MODMACHINE_H
#define MICROPY_INCLUDED_RPI_MODMACHINE_H

typedef struct _machine_pin_obj_t {
    const mp_obj_base_t base;
    const uint32_t id;
    uint32_t pull_mode; // to save pull up/down settings
} machine_pin_obj_t;

extern const mp_obj_type_t machine_pin_type;
extern const mp_obj_type_t machine_i2c_type;
extern const mp_obj_type_t machine_spi_type;
extern const mp_obj_type_t machine_pwm_type;

#endif // MICROPY_INCLUDED_RPI_MODMACHINE_H
