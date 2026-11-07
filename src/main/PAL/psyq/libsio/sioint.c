/* RODATA 0x80012108 0x4c */
/* GCC_VERSION: 970318 */
/* MASPSX_FLAGS: --assembler-profile=gnu */
#include "types.h"
#include "rr/serial.h"
#include "rr/interrupts.h"
#include "m2c_macros.h"
#include "psyq/libsio/siorecv.h"
#include "psyq/libsio/siosend.h"
#include "psyq/libsio/siostart.h"
#include "psyq/libsio/siowait.h"

/* ASM_BARRIER: retain a constant data base in a register so its fields use base offsets. */
#define KEEP_BASE(p) __asm__ volatile("" : "=r"(p) : "0"(p))

typedef struct { short p0, p2, p4, p6, w8; unsigned short wA; short pC; unsigned short wE; } W5A;

extern s32 ResetGraph(s32);
extern s32 EnterCriticalSection(void);
extern void ExitCriticalSection(void);
extern RrSerialPortRegisters SioShadowRegs;
extern volatile s32 SioPollHook;
extern s16 SioRxIrqModeBits[];
extern volatile s32 *SioDataReg;
extern s32 DeliverEvent(u32, s32);
extern void SysEnqIntRP();
extern void SysDeqIntRP();
extern char SioIrqQueueEntry[];
extern u16 SioShadowControl;

s32 _sio_control(u32 arg0, u32 arg1, u32 arg2);
s32 _sio_irq_handler(void);
int _sio_init(void);
s32 _sio_open(void);
s32 _sio_close(void);
int _sio_inout();
s32 _sio_unsupported(void);

s32 _sio_control(u32 arg0, u32 arg1, u32 arg2) {
    s32 ret = 0;

    switch (arg0) {
    case 0:
        switch (arg1) {
        case 0:
            ret = SIO1_REGS->status;
            break;
        case 1:
            ret = (SIO1_REGS->control >> 1) & 1;
            if (SIO1_REGS->control & 0x20) {
                ret |= 2;
            }
            break;
        case 2:
            ret = SioShadowRegs.mode;
            break;
        case 3:
            ret = 0x1FA400 / SioShadowRegs.baud;
            break;
        case 4:
            ret = 1;
            break;
        case 5:
            if (arg2 == 0) {
                ret = *(volatile s32 *)&SioReceiveRequest.request.second;
            } else {
                ret = SioRequest.second;
            }
            break;
        case 6: {
            volatile RrSerialRequest *req = &SioRequest;
            if (arg2 == 0) {
                req = &SioReceiveRequest.request;
            }
            ret = req->active != 0;
            break;
        }
        }
        break;
    case 1:
        switch (arg1) {
        case 0:
            break;
        case 1: {
            /* ASM_BARRIER: pin the port pointer to $v1 like the original allocation */
            register RrSerialPortRegisters *p asm("$3") = &SioShadowRegs;
            s32 dtr;
            u32 dtr_copy;
            u32 ctrl;
            u32 v;
            p->control = SIO1_REGS->control & ~0x22;
            dtr = (arg2 & 1) << 1;
            dtr_copy = dtr;
            ctrl = p->control;
            if (arg2 & 2) {
                v = ctrl | 0x20;
                v |= dtr;
            } else {
                v = ctrl | dtr_copy;
            }
            p->control = v;
            SIO1_REGS->control = SioShadowRegs.control;
            break;
        }
        case 2: {
            RrSerialPortRegisters *p = &SioShadowRegs;
            p->mode = arg2;
            SIO1_REGS->mode = p->mode;
            break;
        }
        case 3: {
            RrSerialPortRegisters *p;
            if ((0x1FA400 % arg2) != 0) {
                ret = -1;
                break;
            }
            p = &SioShadowRegs;
            p->baud = 0x1FA400 / arg2;
            SIO1_REGS->baud = p->baud;
            SIO1_REGS->control |= 0x10;
            break;
        }
        case 4: {
            RrSerialPortRegisters *p;
            if (arg2 >= 1 && arg2 <= 8) {
                ret = SioRxIrqModeBits[arg2];
                if (ret < 0) {
                    break;
                }
                p = &SioShadowRegs;
                p->control &= ~0x300;
                p->control |= ret;
                SIO1_REGS->control = p->control;
            }
            break;
        }
        }
        break;
    case 2:
        switch (arg1) {
        case 0: {
            s32 v = EnterCriticalSection();
            RrSerialPortRegisters *port = SIO1_REGS;
            port->control |= 0x50;
            port->mode = SioShadowRegs.mode;
            port->control = SioShadowRegs.control;
            port->baud = SioShadowRegs.baud;
            port->control |= 0x10;
            ret = 0;
            if (v == 1) {
                ExitCriticalSection();
            }
            break;
        }
        case 1:
            ret = 0;
            SIO1_REGS->control |= 0x10;
            break;
        case 2: {
            s32 v = EnterCriticalSection();
            SIO1_REGS->control &= ~0x400;
            SioReceiveRequest.request.active = 0;
            SioReceiveRequest.request.second = 0;
            if (v == 1) {
                ExitCriticalSection();
            }
            break;
        }
        case 3: {
            s32 v = EnterCriticalSection();
            SIO1_REGS->control &= ~0x820;
            SioRequest.active = 0;
            SioRequest.second = 0;
            if (v == 1) {
                ExitCriticalSection();
            }
            break;
        }
        }
        break;
    case 3:
        switch (arg1) {
        case 0:
            if (arg2 != 0) {
                SIO1_REGS->control |= 0x20;
            } else {
                SIO1_REGS->control &= ~0x20;
            }
            break;
        case 1:
            if (SIO1_REGS->status & 0x100) {
                ret = 1;
            }
            break;
        }
        break;
    case 4:
        if (arg1 == 0) {
            volatile s32 *q = &SioPollHook;
            ret = *q;
            *(s32 *)q = arg2;
        }
        break;
    case 5:
        if (arg1 == 0) {
            ResetGraph(5);
        }
        break;
    }
    return ret;
}

s32 _sio_irq_check(void) {
    RrInterruptRegisters *interrupts=I_STAT_Sio;
    /* ASM_BARRIER: Keep the predicate result in the common return register. */
    register s32 result;
    if(interrupts->pending & interrupts->mask & 0x100) {
        RrSerialReceiveRequest *request=&SioReceiveRequest;
        /* ASM_BARRIER: Keep the original address materialization before the active load. */
        __asm__("" : "+r"(request));
        if(request->request.active) {
            RrSerialPortRegisters *port=SIO1_REGS;
            if(!(port->status & 1)) {
                result=1;
                goto done;
            }
            port->control &= 0xFBFF;
        }
        result=1;
    } else result=0;
done:
    /* ASM_BARRIER: Keep the selected result before the shared return instruction. */
    __asm__ volatile("" : : "r"(result));
    return result;
}

s32 _sio_irq_handler(void) {
    s32 count;

    if (I_STAT_Sio->pending & 0x100) {
        I_STAT_Sio->pending = ~0x100;
    }
    if (SIO1_REGS->status & 0x38) {
        volatile u16 *cfg;
        volatile RrSerialRequest *req;

        SIO1_REGS->control &= ~0x820;
        cfg = &SioShadowRegs.mode;
        /* ASM_BARRIER: keep the configuration base for its mode/baud fields. */
        KEEP_BASE(cfg);
        req = &SioRequest;
        /* ASM_BARRIER: keep the send request base for both cleared fields. */
        KEEP_BASE(req);
        req->second = 0;
        req->active = 0;
        SIO1_REGS->control;
        SIO1_REGS->control = 0x50;
        SIO1_REGS->mode = cfg[0];
        SIO1_REGS->baud = cfg[3];
        *(u16 *)&SIO1_REGS->control = SIO1_REGS->control | 0x10;
        DeliverEvent(0xF000000B, 0x8000);
        return 0;
    }
    {
        volatile RrSerialRequest *req = &SioRequest;
        /* ASM_BARRIER: keep the send request base for the active/second checks. */
        KEEP_BASE(req);
        if (req->active != 0 && req->second != 0 && (SIO1_REGS->status & 2)) {
            s32 index = (SioShadowRegs.control & 0x300) >> 7;
            /* ASM_BARRIER: finish the table index before the table page lui (original assembler macro order). */
            __asm__ volatile("" : : "r"(index));
            count = 0x800B0000;
            /* ASM_BARRIER: build the size table address in the result register (lui/addu/lh %lo). */
            KEEP_BASE(count);
            count += index;
            count = *(s16 *)(count - 0x42B8);
            switch (count) {
            case 1:
                *(u8 *)SioRequest.first = SIO1_REGS->data;
                break;
            case 2:
                *(u8 *)SioRequest.first = SIO1_REGS->data;
                *((u8 *)SioRequest.first + 1) = SIO1_REGS->data;
                break;
            case 4:
                *(s32 *)SioRequest.first = *SioDataReg;
                break;
            case 8:
                *(s32 *)SioRequest.first = *SioDataReg;
                *((s32 *)SioRequest.first + 1) = *SioDataReg;
                break;
            }
            {
                volatile RrSerialRequest *tx = &SioRequest;
                /* ASM_BARRIER: keep the send request base for the count update. */
                KEEP_BASE(tx);
                tx->first += count;
                tx->second -= count;
                if (tx->second == 0) {
                    tx->active = 0;
                    *(u16 *)&SIO1_REGS->control = SIO1_REGS->control & ~0x820;
                    DeliverEvent(0xF000000B, 0x400);
                }
            }
            SIO1_REGS->control ^= 2;
        }
    }
    {
        volatile RrSerialReceiveRequest *rx = (volatile RrSerialReceiveRequest *)&SioReceiveRequest;
        /* ASM_BARRIER: keep the receive request base for its fields. */
        KEEP_BASE(rx);
        if (rx->request.active != 0 && (SIO1_REGS->status & 1) &&
            ((SIO1_REGS->status & 0x80) == rx->receive_status)) {
            if (rx->request.second == 0) {
                rx->request.active = 0;
                DeliverEvent(0xF000000B, 0x800);
            } else {
                rx->receive_status ^= 0x80;
                SIO1_REGS->data = *(u8 *)SioReceiveRequest.request.first;
                rx->request.first += 1;
                rx->request.second -= 1;
                SIO1_REGS->control |= 0x400;
            }
        }
    }
    SIO1_REGS->control |= 0x10;
    return 0;
}

/* legacy typing of shared globals in this part of the unit */
#define SioShadowRegs (*(W5A (*))&SioShadowRegs)
#define SIO1_REGS ((W5A *)SIO1_REGS)
#define I_STAT_Sio ((volatile int *)I_STAT_Sio)
#define SioRequest (*(int (*)[3])&SioRequest)
#define SioReceiveRequest (*(int (*)[3])&SioReceiveRequest)

int _sio_init(void) {
    W5A *a = &SioShadowRegs;
    volatile W5A *b = SIO1_REGS;
    char *s;
    int r;
    int t;
    volatile int *d;
    volatile int *e;
    a->wA = 5;
    a->w8 = 0xCE;
    a->wE = 0xD8;
    b->wA = 0x50;
    b->w8 = a->w8;
    b->wE = a->wE;
    b->wA |= 0x10;
    ((W5A *)b)->wA = a->wA;
    s = SioIrqQueueEntry;
    SysDeqIntRP(3, s);
    SysEnqIntRP(3, s);
    I_STAT_Sio[1] |= 0x100;
    e = SioRequest;
    d = SioReceiveRequest;
    e[0] = 0;
    d[0] = e[0];
    e[1] = 0;
    d[1] = e[1];
    e[2] = 0;
    t = e[2];
    SioPollHook = 0;
    /* ASM_BARRIER: return constant 0 through a register (original li v0,0 after the last store) */
    __asm__ volatile("" : "=r"(r) : "0"(0));
    d[2] = t;
    return r;
}

#undef SioShadowRegs
#undef SIO1_REGS
#undef I_STAT_Sio
#undef SioRequest
#undef SioReceiveRequest

s32 _sio_open(void) {
    return 0;
}

s32 _sio_close(void) {
    return 0;
}

/* legacy typing of shared globals in this part of the unit */
#define SioIrqQueueEntry (*(u8 (*)[])&SioIrqQueueEntry)
#define SysDeqIntRP ((s32 (*)(s32,void *))SysDeqIntRP)

s32 _sio_deinit(void) {
    RrInterruptRegisters *interrupts=I_STAT_Sio;
    RrSerialPortRegisters *port;
    volatile u16 *control;
    interrupts->mask &= ~0x100U;
    port=SIO1_REGS;
    port->control |= 0x50;
    control=&SioShadowControl;
    /* The final control write occupies the original call's delay slot. */
    *(u16 *)&port->control=*control;
    SysDeqIntRP(3,SioIrqQueueEntry);
    {
        /* Share the request bases across the store/reload pairs. */
        register volatile RrSerialRequest *send=&SioRequest;
        register RrSerialReceiveRequest *receive;
        /* ASM_BARRIER: Reload request words through the original value register. */
        register s32 active __asm__("$3");
        /* ASM_BARRIER: Keep zero in the shared return register after the sender base expires. */
        register s32 result __asm__("$2");
        /* ASM_BARRIER: Retain the request base across both store/reload pairs. */
        __asm__("" : "=r"(send) : "0"(send));
        send->active=0;
        active=send->active;
        /* ASM_BARRIER: Finish the active reload before materializing the receiver base. */
        __asm__ volatile("" : : "r"(active) : "$4", "memory");
        receive=&SioReceiveRequest;
        /* ASM_BARRIER: Materialize the receive prefix before publishing its reloaded fields. */
        __asm__("" : "=r"(receive) : "0"(receive));
        receive->request.active=active;
        send->second=0;
        active=send->second;
        result=0;
        receive->request.second=active;
        /* ASM_BARRIER: Keep the zero result before the shared return instruction. */
        __asm__ volatile("" : : "r"(result));
        return result;
    }
}

#undef SioIrqQueueEntry
#undef SysDeqIntRP

int _sio_inout(a0, a1)
    int *a0;
    int a1;
{
    int v;
    a0[6] = 0;
    if (a1 == 1) {
        if (a0[0] & 0x8000) {
            if (_sio_write_async(a0[2], a0[3]) != 0) {
                a0[6] = 0x10;
                return -1;
            }
            return 0;
        }
        v = _sio_write(a0[2], a0[3]);
        if (v < 0) a0[6] = 5;
        return v;
    }
    if (a1 == 2) {
        if (a0[0] & 0x8000) {
            if (_sio_read_async(a0[2], a0[3]) == 0) return 0;
            a0[6] = 0x10;
            return -1;
        }
        v = _sio_read(a0[2], a0[3]);
        if (v < 0) a0[6] = 5;
        return v;
    }
    a0[6] = 0x16;
    return -1;
}

s32 _sio_unsupported(void) {
    return 0;
}
