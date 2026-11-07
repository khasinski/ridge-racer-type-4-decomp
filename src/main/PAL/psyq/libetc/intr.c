/* GCC_VERSION: 970318 */
/* MASPSX_FLAGS: --assembler-profile=gnu */
#include "types.h"
#include "rr/timing.h"
#include "rr/library.h"
#include "rr/spu_tick_control.h"
#include "m2c_macros.h"
#include "rr/interrupts.h"
#include "rr/overlay668_stream.h"

extern volatile unsigned *GPUSTAT;
extern volatile unsigned *RCNT1_VALUE;
extern volatile unsigned VSyncRcnt1Last;
extern volatile unsigned VSyncLastVcount;
extern s32 PsyqPuts(const char *, ...);
extern s32 ChangeClearPAD(s32);
extern s32 ChangeClearRCnt(s32,s32);
extern u16 IntrEnvInInterrupt;
extern u16 *I_MASK;

void v_wait(s32 a0, s32 a1);
RrLibInterruptCallback InterruptCallback(s32 channel, RrLibInterruptCallback callback);
RrLibInterruptCallback DMACallback(s32 channel, RrLibInterruptCallback callback);
void VSyncCallbacks();
void StopCallback();
void RestartCallback();
u16 CheckCallback(void);
u16 GetIntrMask(void);

int VSync(a0) int a0; {
    volatile unsigned t;
    unsigned s0;
    unsigned s1;
    volatile unsigned *r;
    register int v __asm__("$2"); /* ASM_BARRIER: first call argument stays in v0 like the original */
    int w;
    s0 = *GPUSTAT;
    do {
        t = *RCNT1_VALUE;
    } while (t != *RCNT1_VALUE);
    s1 = (t - VSyncRcnt1Last) & 0xFFFF;
    if (a0 < 0) return Vcount;
    if (a0 == 1) return s1;
    if (a0 > 0) {
        v = VSyncLastVcount;
        v = v - 1;
        v = v + a0;
    } else {
        v = VSyncLastVcount;
    }
    w = 0;
    if (a0 > 0) w = a0 - 1;
    v_wait(v, w);
    s0 = *GPUSTAT;
    v_wait(Vcount + 1, 1);
    if (s0 & 0x400000) {
        r = GPUSTAT;
        if ((int)(s0 ^ *r) >= 0) {
            do {
            } while (((s0 ^ *r) & 0x80000000) == 0);
        }
    }
    VSyncLastVcount = Vcount;
    do {
        VSyncRcnt1Last = *RCNT1_VALUE;
    } while (VSyncRcnt1Last != *RCNT1_VALUE);
    return s1;
}

void v_wait(s32 a0, s32 a1) {
    volatile u32 cnt = (u32)a1 << 15;
    u32 m;
    if ((s32)Vcount < a0) {
        /* ASM_BARRIER: keeps -1 in a register hoisted before the goto loop (original has li v1,-1 before the loop label) */
        __asm__("" : "=r"(m) : "0"(-1));
loop:
        cnt=cnt-1;
        if (cnt == m) {
            char *page=(char *)0x80010000U;
            /* ASM_BARRIER: Materialize the message page before its call-delay offset. */
            __asm__("" : "=r"(page) : "0"(page));
            PsyqPuts(page+0x17F8);
            ChangeClearPAD(0);
            ChangeClearRCnt(3, 0);
            return;
        }
        if ((s32)Vcount < a0) goto loop;
    }
}

void ResetCallback(void) {
    IntrDriverTablePtr->reset();
}

RrLibInterruptCallback InterruptCallback(s32 channel, RrLibInterruptCallback callback) {
    return IntrDriverTablePtr->install(channel, callback);
}

RrLibInterruptCallback DMACallback(s32 channel, RrLibInterruptCallback callback) {
    return IntrDriverTablePtr->exchange(channel,callback);
}

void VSyncCallback(RrSpuTickCallback callback)
{
    ((RrSpuTickInterruptDriver20 *)IntrDriverTablePtr)->install14(4,callback);
}

/* legacy typing of shared globals in this part of the unit */
#define IntrDriverTablePtr (*(int (**(*))())&IntrDriverTablePtr)

void VSyncCallbacks() { IntrDriverTablePtr[5](); }

#undef IntrDriverTablePtr

/* legacy typing of shared globals in this part of the unit */
#define IntrDriverTablePtr (*(int (**(*))())&IntrDriverTablePtr)

void StopCallback() { IntrDriverTablePtr[4](); }

#undef IntrDriverTablePtr

/* legacy typing of shared globals in this part of the unit */
#define IntrDriverTablePtr (*(int (**(*))())&IntrDriverTablePtr)

void RestartCallback() { IntrDriverTablePtr[6](); }

#undef IntrDriverTablePtr

u16 CheckCallback(void) {
    return IntrEnvInInterrupt;
}

u16 GetIntrMask(void) {
    return *I_MASK;
}

s32 SetIntrMask(s32 value) {
    s32 previous=*I_MASK;
    *I_MASK=value;
    return previous;
}
