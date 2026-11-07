/* GCC_VERSION: 2.7.2 */
#include "rr/library.h"
extern void StClearRing(void);
void StSetRing(RrLibQueueRecord32 *first, s32 second) {
    /* ASM_BARRIER: preserve the original assembler temporary for the mapped store. */
    register s32 *base __asm__("$1");
    StRingHeaders = first;
    /* ASM_BARRIER: finish the preceding state updates before binding the address. */
    __asm__ volatile("" : : : "$1", "memory");
    /* PAL RAM mapping for D_80110314. */
    base = (s32 *)0x80110000U;
    /* ASM_BARRIER: allow the register-relative store in the branch delay slot. */
    __asm__("" : "=r"(base) : "0"(base));
    base[0x314 / 4] = second;
    StClearRing();
}
