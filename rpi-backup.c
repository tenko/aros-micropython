#include "rpi.h"
#define asm __asm__
#include <stdio.h>
#include <string.h>

#include <exec/execbase.h>
#include <exec/types.h>
#include <exec/io.h>
#include <exec/memory.h>
#include <devices/timer.h>

#include <proto/exec.h>
#include <clib/alib_protos.h>
#include <proto/kernel.h>
#include <proto/mbox.h>

#include <hardware/bcm2708.h>
#include <hardware/videocore.h>

/* BCM2712 moved the mailbox within the peripheral window. */
#define VCMB_OFFSET_BCM2712             0x013880
#define RPI_VCMB_BASE(peri)             ((void *)(IPTR)((peri) + \
    (((peri) == BCM2712_PERIIOBASE) ? VCMB_OFFSET_BCM2712 : VCMB_OFFSET)))

uint32_t periiobase;
static uint32_t freq_cpu  = 700000000;
static uint32_t freq_core = 250000000;

struct Library *TimerBase = NULL;
struct timerequest GlobalTimeReq;

/* Shared helper used by the AROS semaphore implementation too. */
void InitQPort(struct MsgPort *port)
{
    /* The callers' ports live on their stacks: clear the whole structure so
       the node and the message list lock start out valid rather than as
       leftover stack contents */
    memset(port, 0, sizeof(struct MsgPort));
    port->mp_Node.ln_Type = NT_MSGPORT;
    port->mp_SigBit = SIGB_SINGLE;
    port->mp_Flags = PA_SIGNAL;
    port->mp_SigTask = FindTask(NULL);
    NEWLIST(&port->mp_MsgList);
}

void aros_delay_ns(uint64_t ns)
{
    struct timerequest req;
    struct MsgPort port;

    InitQPort(&port);

    req.tr_node.io_Message.mn_Node.ln_Type = 0;
    req.tr_node.io_Message.mn_ReplyPort = &port;
    req.tr_node.io_Device = GlobalTimeReq.tr_node.io_Device;
    req.tr_node.io_Unit = GlobalTimeReq.tr_node.io_Unit;
    req.tr_node.io_Command = TR_ADDREQUEST;
    req.tr_time.tv_secs = ns / 1000000000ULL;
    req.tr_time.tv_micro = (ns / 1000) % 1000000;

    DoIO((struct IORequest *)&req);
}

void aros_delay_us(uint32_t delay) {
    aros_delay_ns(1000*delay);
}

static UQUAD vcQueryClock(ULONG tag, ULONG reg)
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
    msg[2] = AROS_LONG2LE(tag);
    msg[3] = AROS_LONG2LE(2 * sizeof(unsigned int));    /* value buffer size */
    msg[4] = 0;                                         /* req/resp code */
    msg[5] = AROS_LONG2LE(reg);
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
    return freq_cpu;
}

void rpi_init() {
    APTR KernelBase = OpenResource((const unsigned char *)"kernel.resource");
    periiobase = (uint32_t)(KernelBase ? (IPTR)KrnGetSystemAttr(KATTR_PeripheralBase) : 0);
    freq_cpu = vcQueryClock(VCTAG_GETCLKRATE, VCCLOCK_ARM);
    freq_core = vcQueryClock(VCTAG_GETCLKRATE, VCCLOCK_CORE);
    printf("periiobase = %X\n", periiobase);
    printf("arm clock = %lu\n", freq_cpu);
    printf("core clock = %lu\n", freq_core);
    if (OpenDevice("timer.device", UNIT_MICROHZ, &GlobalTimeReq.tr_node, 0) == 0)
	{
		TimerBase = (struct Library *)GlobalTimeReq.tr_node.io_Device;
	}
}

void rpi_deinit() {
    if (TimerBase != NULL) {
        CloseDevice((struct IORequest *)&GlobalTimeReq);
    }
}
