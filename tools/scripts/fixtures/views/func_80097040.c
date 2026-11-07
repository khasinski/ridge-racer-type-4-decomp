/* GCC_VERSION: 2.7.2 */

#include "rr/counters.h"

/* legacy typing of shared globals in this part of the unit */
#define I_STAT_RCnt ((u32 *)I_STAT_RCnt)

#undef I_STAT_RCnt

s32 GetRCnt(u32 id) {
    s32 index=id & 0xFFFF;
    s32 result;
    if(index<3) {
        u16 count=RCNT_REGS[index].count;
        /* ASM_BARRIER: Preserve the zero-extended halfword load as the return word. */
        __asm__("" : "=r"(result) : "0"(count));
    }
    else result=0;
    return result;
}
