/* GCC_VERSION: 970318 */

/* MASPSX_FLAGS: --assembler-profile=gnu */

#include "rr/serial.h"

/* ASM_BARRIER: retain a constant data base in a register so its fields use base offsets. */
#define KEEP_BASE(p) __asm__ volatile("" : "=r"(p) : "0"(p))

/* legacy typing of shared globals in this part of the unit */
#define SioShadowRegs (*(W5A (*))&SioShadowRegs)

#define SIO1_REGS ((W5A *)SIO1_REGS)

#define I_STAT_Sio ((volatile int *)I_STAT_Sio)

#define SioRequest (*(int (*)[3])&SioRequest)

#define SioReceiveRequest (*(int (*)[3])&SioReceiveRequest)

#undef SioShadowRegs

#undef SIO1_REGS

#undef I_STAT_Sio

#undef SioRequest

#undef SioReceiveRequest

/* legacy typing of shared globals in this part of the unit */
#define SioIrqQueueEntry (*(u8 (*)[])&SioIrqQueueEntry)

#define SysDeqIntRP ((s32 (*)(s32,void *))SysDeqIntRP)

#undef SioIrqQueueEntry

#undef SysDeqIntRP

extern s32 ResetGraph(s32);

extern s32 EnterCriticalSection(void);

extern void ExitCriticalSection(void);

extern RrSerialPortRegisters SioShadowRegs;

extern volatile s32 SioPollHook;

extern s16 SioRxIrqModeBits[];

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
