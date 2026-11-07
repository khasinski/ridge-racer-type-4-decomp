/* GCC_VERSION: 970318 */

/* MASPSX_FLAGS: --assembler-profile=gnu */

#include "rr/timing.h"

/* legacy typing of shared globals in this part of the unit */
#define IntrDriverTablePtr (*(int (**(*))())&IntrDriverTablePtr)

#undef IntrDriverTablePtr

/* legacy typing of shared globals in this part of the unit */
#define IntrDriverTablePtr (*(int (**(*))())&IntrDriverTablePtr)

#undef IntrDriverTablePtr

/* legacy typing of shared globals in this part of the unit */
#define IntrDriverTablePtr (*(int (**(*))())&IntrDriverTablePtr)

#undef IntrDriverTablePtr

extern volatile unsigned *GPUSTAT;

extern volatile unsigned *RCNT1_VALUE;

extern volatile unsigned VSyncRcnt1Last;

extern volatile unsigned VSyncLastVcount;

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
