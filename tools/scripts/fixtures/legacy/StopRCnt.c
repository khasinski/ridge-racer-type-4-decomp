/* GCC_VERSION: 2.7.2 */
#include "types.h"
/* SDK view of the interrupt registers at 0x1F801070. */
extern u32 *D_800ABC30;
s32 StopRCnt(u32 index) {
    u32 offset = (u32)(u16)index * 4;
    u32 *state;
    u8 *base;
    state = D_800ABC30;
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
