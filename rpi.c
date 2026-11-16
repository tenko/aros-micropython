#include "rpi.h"
#define asm __asm__
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include <proto/exec.h>
#include <proto/kernel.h>
#include <proto/mbox.h>

#include <hardware/bcm2708.h>
#include <hardware/videocore.h>

#include "bcm283x.h"
#include "bcm283x_systimer.h"

/* BCM2712 moved the mailbox within the peripheral window. */
#define VCMB_OFFSET_BCM2712             0x013880
#define RPI_VCMB_BASE(peri)             ((void *)(IPTR)((peri) + \
    (((peri) == BCM2712_PERIIOBASE) ? VCMB_OFFSET_BCM2712 : VCMB_OFFSET)))

uint32_t periiobase;
static uint32_t freq_cpu  = 700000000;
static uint32_t freq_core = 250000000;

void aros_disable(void) {
    Disable();
}

void aros_enable(void) {
    Enable();
}

// System Timers

systimer_t *systimer;

volatile uint64_t rpi_systime(void) {
    uint64_t t;
    uint32_t chi;
    uint32_t clo;

    chi = systimer->CHI;
    clo = systimer->CLO;
    if (chi != systimer->CHI) {
        chi = systimer->CHI;
        clo = systimer->CLO;
    }
    t = chi;
    t = t << 32;
    t += clo;
    return t;
}

volatile uint64_t rpi_elapsed_from(uint64_t t) {
    uint64_t now;
    uint32_t chi;
    uint32_t clo;

    now = rpi_systime();
    if (now > t) {
        return now - t;
    } else {
        return ULLONG_MAX - t + now;
    }

    chi = systimer->CHI;
    clo = systimer->CLO;
    if (chi != systimer->CHI) {
        chi = systimer->CHI;
        clo = systimer->CLO;
    }
    t = chi;
    t = t << 32;
    t += clo;
    return t;
}


static UQUAD vcQueryCoreClock(void)
{
    APTR MBoxBase = NULL;
    unsigned int *raw, *msg;
    UQUAD rate = 0;

    if ((MBoxBase = OpenResource((const unsigned char *)"mbox.resource")) == NULL)
        return 0;

    raw = AllocMem(MBOX_MSG_ALIGN + (MBOX_MSG_ALIGN - 1), MEMF_PUBLIC | MEMF_CLEAR);
    if (!raw)
        return 0;
    msg = (unsigned int *)((((IPTR)raw) + (MBOX_MSG_ALIGN - 1)) & ~(IPTR)(MBOX_MSG_ALIGN - 1));

    msg[0] = AROS_LONG2LE(8 * sizeof(unsigned int));
    msg[1] = AROS_LONG2LE(VCTAG_REQ);
    msg[2] = AROS_LONG2LE(VCTAG_GETCLKRATE);
    msg[3] = AROS_LONG2LE(2 * sizeof(unsigned int));    /* value buffer size */
    msg[4] = 0;                                         /* req/resp code */
    msg[5] = AROS_LONG2LE(VCCLOCK_CORE);
    msg[6] = 0;
    msg[7] = 0;                                         /* end tag */

    if (MBoxCall((void *)RPI_VCMB_BASE(periiobase), VCMB_PROPCHAN, msg) == (volatile unsigned int *)msg &&
            (AROS_LE2LONG(msg[4]) & VCTAG_RESP))
    {
        rate = AROS_LE2LONG(msg[6]);
    }
    FreeMem(raw, MBOX_MSG_ALIGN + (MBOX_MSG_ALIGN - 1));
    return rate;
}

// Clock speed

uint32_t rpi_freq_core() {
    return freq_core;
}

uint32_t rpi_freq_cpu() {
    return freq_core;
}

void rpi_init() {
    APTR KernelBase = OpenResource((const unsigned char *)"kernel.resource");
    periiobase = (uint32_t)(KernelBase ? (IPTR)KrnGetSystemAttr(KATTR_PeripheralBase) : 0);
    printf("periiobase = %X\n", periiobase);
    freq_core = vcQueryCoreClock();
    printf("core clock = %u\n", freq_core);
    systimer = (systimer_t *) SYSTIMER;
}

void rpi_deinit() {
}
