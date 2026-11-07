/* GCC_VERSION: 2.7.2 */
/* MASPSX_FLAGS: --assembler-profile=gnu */
#include "psyq/libsio/siorecv.h"
#include "psyq/libsio/siosend.h"

typedef struct {
    volatile unsigned char data;
    unsigned char pad01[3];
    volatile unsigned short status;
    unsigned char pad06[2];
    volatile unsigned short mode;
    volatile unsigned short control;
    unsigned char pad0C[2];
    volatile unsigned short baud;
} RrSerialPortRegisters;

typedef struct {
    int active;
    int first, second;
} RrSerialRequest;

extern volatile RrSerialRequest SioRequest;
extern RrSerialPortRegisters *SIO1_REGS;
extern RrSerialPortRegisters SioShadowRegs;
extern int (*SioPollHook)(int, int);
extern int DeliverEvent(unsigned int, int);

int _sio_write(int first, int second) {
    int s1 = 0;
    int s0;
    int s4;
    RrSerialPortRegisters *port;
    /* ASM_BARRIER: retain the request base in $a1 before the loop. */
    volatile RrSerialRequest *p1 = &SioRequest;

    /* ASM_BARRIER: keep $a1 live for the pre-loop stores and load. */
    __asm__("" : "=r"(p1) : "0"(p1));

    if (p1->active != 0) {
        return -1;
    }

    s4 = SioRxFifoSizeTable[(SioShadowRegs.control & 0x300) >> 8];
    port = SIO1_REGS;
    p1->second = second;
    p1->first = first;
    p1->active = 0;
    port->control |= 0x20;
    s0 = 0;

    if (p1->second != 0) {
        volatile RrSerialRequest *b = p1;
        do {
            RrSerialPortRegisters *p = SIO1_REGS;
            if ((p->status & 0x38) != 0) {
                int v50 = 0x50;
                unsigned short ctrl;
                volatile unsigned short *cfg;
                /* ASM_BARRIER: materialize 0x50 at the start of the error block for the branch delay slot. */
                __asm__ volatile("" : "=r"(v50) : "0"(v50));
                ctrl = p->control;
                /* ASM_BARRIER: schedule the control load before the configuration base load and keep 0x50 in $v0. */
                __asm__ volatile("" : : "r"(ctrl), "r"(v50), "r"(v50));
                cfg = &SioShadowRegs.mode;
                /* ASM_BARRIER: keep the configuration base in $v1 for mode and baud loads. */
                __asm__("" : "=r"(cfg) : "0"(cfg));
                p->control = v50;
                p->mode = cfg[0];
                p->baud = cfg[3];
                p->control |= 0x10;
                p->control = ctrl;
                p->control &= ~0x20;
                DeliverEvent(0xF000000B, 0x8000);
                goto done;
            }
            if ((p->status & 2) == 0) {
                do {
                    /* ASM_BARRIER: weight $s0 and $s1 counters ahead of request base $s2 in global-alloc. */
                    __asm__ volatile("" : : "r"(s0), "r"(s0), "r"(s1), "r"(s1));
                    if (SioPollHook != 0) {
                        if (SioPollHook(1, s1++) == 0) {
                            SIO1_REGS->control &= ~0x20;
                            DeliverEvent(0xF000000B, 0x100);
                            goto done;
                        }
                    }
                } while ((SIO1_REGS->status & 2) == 0);
            }
            *(unsigned char *)SioRequest.first = SIO1_REGS->data;
            b->first += 1;
            s0 += 1;
            b->second -= 1;
            if (s0 == s4) {
                s0 = 0;
                SIO1_REGS->control ^= 2;
            }
        } while (b->second != 0);
    }
    SIO1_REGS->control &= ~0x20;
done:
    /* ASM_BARRIER: weight second argument into $s3 ahead of chunk size $s4 in global-alloc. */
    __asm__ volatile("" : : "r"(second));
    return second - SioRequest.second;
}
