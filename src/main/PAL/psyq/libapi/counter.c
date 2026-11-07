/* GCC_VERSION: 2.7.2 */
#include "types.h"
#include "rr/counters.h"
#include "rr/spu_tick_control.h"

extern int *I_STAT_RCnt;
extern int RCntIntrMaskBits[];

int SetRCnt(unsigned a0, unsigned short a1, int a2);
int StartRCnt();
s32 StopRCnt(u32 index);

int SetRCnt(unsigned a0, unsigned short a1, int a2) {
    int t = a0 & 0xFFFF;
    int a3 = 0x48;
    char *p;
    if (t >= 3) return 0;
    p = (char *)(t * 16 + (unsigned long)RCNT_REGS);
    *(short *)(p + 4) = 0;
    *(volatile short *)(p + 8) = a1;
    if ((unsigned)t < 2) {
        if (a2 & 0x10) a3 = 0x49;
        if (!(a2 & 1)) a3 |= 0x100;
    } else {
        if (t == 2) {
            if (!(a2 & 1)) a3 = 0x248;
        }
    }
    if (a2 & 0x1000) a3 |= 0x10;
    *(short *)(t * 16 + (unsigned long)RCNT_REGS + 4) = a3;
    return 1;
}

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

int StartRCnt(a0) unsigned int a0; {
  int v = a0 & 0xFFFF;
  int *p = I_STAT_RCnt;
  p[1] |= RCntIntrMaskBits[v];
  return v < 3;
}

/* legacy typing of shared globals in this part of the unit */
#define I_STAT_RCnt ((u32 *)I_STAT_RCnt)

s32 StopRCnt(u32 index) {
    u32 offset = (u32)(u16)index * 4;
    u32 *state;
    u8 *base;
    state = I_STAT_RCnt;
    /* ASM_BARRIER: load the register pointer before materializing the mask table base. */
    __asm__ volatile("" : : "r"(state), "r"(offset) : "$2");
    /* PAL RAM mapping: RCntIntrMaskBits is base minus 0x43C8. */
    base = (u8 *)0x800B0000U;
    /* ASM_BARRIER: preserve the explicit base-plus-offset table load. */
    __asm__("" : "=r"(base) : "0"(base));
    base += offset;
    {
        /* ASM_BARRIER: reuse v0 for the mask, its complement and the return value. */
        register u32 mask __asm__("$2") = *(u32 *)(base - 0x43C8);
        mask = ~mask;
        state[1] &= mask;
    }
    return 1;
}

#undef I_STAT_RCnt

int ResetRCnt(a0) unsigned int a0; {
  int v = a0 & 0xFFFF;
  if (v >= 3) return 0;
  *(short *)((u8 *)RCNT_REGS + v * 16) = 0; return 1;
}
