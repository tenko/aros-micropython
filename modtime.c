/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2023 Damien P. George
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
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <stdio.h>

#include "py/mphal.h"
#include "py/runtime.h"
#include "shared/timeutils/timeutils.h"

static mp_obj_t mp_time_time_get(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    mp_float_t val = tv.tv_sec + (mp_float_t)tv.tv_usec / 1000000;
    return mp_obj_new_float(val);
}

static mp_obj_t mp_time_sleep(mp_obj_t arg) {
    struct timeval tv;
    mp_float_t val = mp_obj_get_float(arg);
    mp_float_t ipart;
    tv.tv_usec = (time_t)MICROPY_FLOAT_C_FUN(round)(MICROPY_FLOAT_C_FUN(modf)(val, &ipart) * MICROPY_FLOAT_CONST(1000000.));
    tv.tv_sec = (suseconds_t)ipart;
    mp_hal_delay_us(tv.tv_sec * 1000000 + tv.tv_usec);
    return mp_const_none;
}

static mp_obj_t mod_time_gm_local_time(size_t n_args, const mp_obj_t *args, struct tm *(*time_func)(const time_t *timep)) {
    time_t t;
    if (n_args == 0) {
        t = time(NULL);
    } else {
        t = (time_t)timeutils_obj_get_timestamp(args[0]);
    }
    struct tm *tm = time_func(&t);

    mp_obj_t ret = mp_obj_new_tuple(9, NULL);

    mp_obj_tuple_t *tuple = MP_OBJ_TO_PTR(ret);
    tuple->items[0] = MP_OBJ_NEW_SMALL_INT(tm->tm_year + 1900);
    tuple->items[1] = MP_OBJ_NEW_SMALL_INT(tm->tm_mon + 1);
    tuple->items[2] = MP_OBJ_NEW_SMALL_INT(tm->tm_mday);
    tuple->items[3] = MP_OBJ_NEW_SMALL_INT(tm->tm_hour);
    tuple->items[4] = MP_OBJ_NEW_SMALL_INT(tm->tm_min);
    tuple->items[5] = MP_OBJ_NEW_SMALL_INT(tm->tm_sec);
    int wday = tm->tm_wday - 1;
    if (wday < 0) {
        wday = 6;
    }
    tuple->items[6] = MP_OBJ_NEW_SMALL_INT(wday);
    tuple->items[7] = MP_OBJ_NEW_SMALL_INT(tm->tm_yday + 1);
    tuple->items[8] = MP_OBJ_NEW_SMALL_INT(tm->tm_isdst);

    return ret;
}

static mp_obj_t mod_time_gmtime(size_t n_args, const mp_obj_t *args) {
    return mod_time_gm_local_time(n_args, args, gmtime);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_time_gmtime_obj, 0, 1, mod_time_gmtime);

static mp_obj_t mod_time_localtime(size_t n_args, const mp_obj_t *args) {
    return mod_time_gm_local_time(n_args, args, localtime);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_time_localtime_obj, 0, 1, mod_time_localtime);

static mp_obj_t mod_time_mktime(mp_obj_t tuple) {
    size_t len;
    mp_obj_t *elem;
    mp_obj_get_array(tuple, &len, &elem);

    // localtime generates a tuple of len 8. CPython uses 9, so we accept both.
    if (len < 8 || len > 9) {
        mp_raise_TypeError(MP_ERROR_TEXT("mktime needs a tuple of length 8 or 9"));
    }

    struct tm time = {
        .tm_year = mp_obj_get_int(elem[0]) - 1900,
        .tm_mon = mp_obj_get_int(elem[1]) - 1,
        .tm_mday = mp_obj_get_int(elem[2]),
        .tm_hour = mp_obj_get_int(elem[3]),
        .tm_min = mp_obj_get_int(elem[4]),
        .tm_sec = mp_obj_get_int(elem[5]),
    };
    if (len == 9) {
        time.tm_isdst = mp_obj_get_int(elem[8]);
    } else {
        time.tm_isdst = -1; // auto-detect
    }
    time_t ret = mktime(&time);
    if (ret == (time_t)-1) {
        mp_raise_msg(&mp_type_OverflowError, MP_ERROR_TEXT("invalid mktime usage"));
    }
    return timeutils_obj_from_timestamp(ret);
}
MP_DEFINE_CONST_FUN_OBJ_1(mod_time_mktime_obj, mod_time_mktime);

#define MICROPY_PY_TIME_EXTRA_GLOBALS \
    { MP_ROM_QSTR(MP_QSTR_gmtime), MP_ROM_PTR(&mod_time_gmtime_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_localtime), MP_ROM_PTR(&mod_time_localtime_obj) }, \
    { MP_ROM_QSTR(MP_QSTR_mktime), MP_ROM_PTR(&mod_time_mktime_obj) },
