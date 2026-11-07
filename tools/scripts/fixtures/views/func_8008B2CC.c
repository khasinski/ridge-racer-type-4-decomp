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

extern s32 PsyqPuts(const char *, ...);

extern s32 ChangeClearPAD(s32), ChangeClearRCnt(s32,s32);

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
