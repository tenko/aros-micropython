/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016-2023 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

// This file is never compiled standalone, it's included directly from
// extmod/machine_uart.c via MICROPY_PY_MACHINE_UART_INCLUDEFILE.

#include "py/mperrno.h"
#include "py/mphal.h"
#include "rpi.h"
#include "mini_uart.h"

typedef struct _machine_uart_obj_t {
    mp_obj_base_t base;
    uint8_t uart_id;
    uint8_t bits;
    uint8_t parity;
    uint8_t stop;
    uint32_t baudrate;
    uint16_t timeout;       // timeout waiting for first char (in ms)
    uint16_t timeout_char;  // timeout waiting between chars (in ms)
} machine_uart_obj_t;

static const char *_parity_name[] = {"None", "1", "0"};

/******************************************************************************/
// MicroPython bindings for UART

// The UART class doesn't have any constants for this port.
#define MICROPY_PY_MACHINE_UART_CLASS_CONSTANTS

static void mp_machine_uart_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_uart_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "UART(%u, baudrate=%u, bits=%u, parity=%s, stop=%u, timeout=%u, timeout_char=%u)",
        self->uart_id, self->baudrate, self->bits, _parity_name[self->parity],
        self->stop, self->timeout, self->timeout_char);
}

static void mp_machine_uart_init_helper(machine_uart_obj_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_baudrate, ARG_bits, ARG_parity, ARG_stop, ARG_tx, ARG_rx, ARG_rxbuf, ARG_timeout, ARG_timeout_char };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_baudrate, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_bits, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_parity, MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_stop, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_tx, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_rx, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_rxbuf, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_timeout, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_timeout_char, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // set baudrate
    if (args[ARG_baudrate].u_int > 0) {
        self->baudrate = args[ARG_baudrate].u_int;
    }

    // set data bits
    switch (args[ARG_bits].u_int) {
        case 0:
            break;
        case 5:
            self->bits = 5;
            break;
        case 6:
            self->bits = 6;
            break;
        case 7:
            self->bits = 7;
            break;
        case 8:
            self->bits = 8;
            break;
        default:
            mp_raise_ValueError(MP_ERROR_TEXT("invalid data bits"));
            break;
    }

    // set parity
    if (args[ARG_parity].u_obj != MP_OBJ_NULL) {
        if (args[ARG_parity].u_obj == mp_const_none) {
            self->parity = 0;
        } else {
            mp_int_t parity = mp_obj_get_int(args[ARG_parity].u_obj);
            if (parity & 1) {
                self->parity = 1;
            } else {
                self->parity = 2;
            }
        }
    }

    // set stop bits
    switch (args[ARG_stop].u_int) {
        case 0:
            break;
        case 1:
            self->stop = 1;
            break;
        case 2:
            self->stop = 2;
            break;
        default:
            mp_raise_ValueError(MP_ERROR_TEXT("invalid stop bits"));
            break;
    }

    // set timeout
    self->timeout = args[ARG_timeout].u_int;

    // set timeout_char
    // make sure it is at least as long as a whole character (13 bits to be safe)
    self->timeout_char = args[ARG_timeout_char].u_int;
    uint32_t min_timeout_char = 13000 / self->baudrate + 1;
    if (self->timeout_char < min_timeout_char) {
        self->timeout_char = min_timeout_char;
    }

    // setup
    mini_uart_init();
    mini_uart_set_speed(self->baudrate);
}

static mp_obj_t mp_machine_uart_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);

    // get uart id
    mp_int_t uart_id = mp_obj_get_int(args[0]);
    if (uart_id != 0 && uart_id != 1) {
        mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("UART(%d) does not exist"), uart_id);
    }

    // create instance
    machine_uart_obj_t *self = mp_obj_malloc(machine_uart_obj_t, &machine_uart_type);
    self->uart_id = uart_id;
    self->baudrate = 115200;
    self->bits = 8;
    self->parity = 0;
    self->stop = 1;
    self->timeout = 0;
    self->timeout_char = 0;

    // init the peripheral
    mp_map_t kw_args;
    mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
    mp_machine_uart_init_helper(self, n_args - 1, args + 1, &kw_args);

    return MP_OBJ_FROM_PTR(self);
}

static void mp_machine_uart_deinit(machine_uart_obj_t *self) {
    (void)self;
}

static mp_int_t mp_machine_uart_any(machine_uart_obj_t *self) {
    return mini_uart_rx_state();
}

static bool mp_machine_uart_txdone(machine_uart_obj_t *self) {
    return mini_uart_txdone();
}

static mp_uint_t mp_machine_uart_read(mp_obj_t self_in, void *buf_in, mp_uint_t size, int *errcode) {
    machine_uart_obj_t *self = MP_OBJ_TO_PTR(self_in);

    // make sure we want at least 1 char
    if (size == 0) {
        return 0;
    }

    // wait for first char to become available
    if (!mini_uart_rx_wait(self->timeout * 1000)) {
        *errcode = MP_EAGAIN;
        return MP_STREAM_ERROR;
    }

    // read the data
    uint8_t *buf = buf_in;
    for (;;) {
        *buf++ = mini_uart_getc();
        if (--size == 0 || !mini_uart_rx_wait(self->timeout_char * 1000)) {
            // return number of bytes read
            return buf - (uint8_t *)buf_in;
        }
    }
}

static mp_uint_t mp_machine_uart_write(mp_obj_t self_in, const void *buf_in, mp_uint_t size, int *errcode) {
    machine_uart_obj_t *self = MP_OBJ_TO_PTR(self_in);
    const byte *buf = buf_in;

    /* TODO implement non-blocking
    // wait to be able to write the first character
    if (!uart_tx_wait(self, timeout)) {
        *errcode = EAGAIN;
        return MP_STREAM_ERROR;
    }
    */

    // write the data
    for (size_t i = 0; i < size; ++i) {
        mini_uart_putc(*buf++);
    }

    // return number of bytes written
    return size;
}

static mp_uint_t mp_machine_uart_ioctl(mp_obj_t self_in, mp_uint_t request, uintptr_t arg, int *errcode) {
    machine_uart_obj_t *self = self_in;
    mp_uint_t ret;
    if (request == MP_STREAM_POLL) {
        mp_uint_t flags = arg;
        ret = 0;
        if ((flags & MP_STREAM_POLL_RD) && mini_uart_rx_state()) {
            ret |= MP_STREAM_POLL_RD;
        }
        if ((flags & MP_STREAM_POLL_WR) && mini_uart_txdone()) {
            ret |= MP_STREAM_POLL_WR;
        }
    } else if (request == MP_STREAM_FLUSH) {
        // The timeout is estimated using the buffer size and the baudrate.
        // Take the worst case assumptions at 13 bit symbol size times 2.
        *errcode = MP_ETIMEDOUT;
        ret = MP_STREAM_ERROR;
    } else {
        *errcode = MP_EINVAL;
        ret = MP_STREAM_ERROR;
    }
    return ret;
}
