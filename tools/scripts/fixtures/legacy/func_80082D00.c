/* GCC_VERSION: 2.7.2 */
#include "types.h"

extern s32 SndSeqTickEnvDividedPhase;
extern void (*SndSeqTickEnvCallback)(void);

void _SsTickCallbackDivided(void) {
    if (SndSeqTickEnvDividedPhase == 0) {
        /* ASM_BARRIER: Retain the assembler temporary for the state publication. */
        register u8 *page __asm__("$1");
        s32 one = 1;
        /* ASM_BARRIER: Materialize the state value before its store address. */
        __asm__ volatile("" : : "r"(one) : "$1", "memory");
        /* PAL mapping: page + 0x4D4C is &SndSeqTickEnvDividedPhase. */
        page = (u8 *)0x800A0000U;
        /* ASM_BARRIER: Expose the PAL store so it can fill the epilogue jump delay slot. */
        __asm__("" : "=r"(page) : "0"(page));
        *(s32 *)(page + 0x4D4C) = one;
    } else {
        /* ASM_BARRIER: Retain the assembler temporary for the state publication. */
        register u8 *page __asm__("$1");
        void (*callback)(void) = SndSeqTickEnvCallback;
        /* ASM_BARRIER: Snapshot the callback before materializing the state-store address. */
        __asm__ volatile("" : : "r"(callback) : "$1", "memory");
        /* PAL mapping: page + 0x4D4C is &SndSeqTickEnvDividedPhase. */
        page = (u8 *)0x800A0000U;
        /* ASM_BARRIER: Expose the PAL store so it can fill the callback call delay slot. */
        __asm__("" : "=r"(page) : "0"(page));
        *(s32 *)(page + 0x4D4C) = 0;
        callback();
    }
}
