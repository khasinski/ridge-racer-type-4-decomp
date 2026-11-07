/* GCC_VERSION: 970318 */

/* MASPSX_FLAGS: --assembler-profile=gnu */

#include "rr/library.h"

/* legacy typing of shared globals in this part of the unit */
#define StLastReadyLoc (*(char (*)[])&StLastReadyLoc)

#undef StLastReadyLoc

extern int StRingStartIndex;

extern int StRingWriteIndex;

extern int StDataReadyPending;

extern s32 StLastReadyFrame;

extern RrLibPackedWord StLastReadyLoc;

extern void (*StDataReadyFunc)(void);

void data_ready_callback(void)
{
    s32 value;
    /* ASM_BARRIER: Keep the next queue index in the original register until publication. */
    register s32 next __asm__("$3");
    /* ASM_BARRIER: Retain the original indirect-call register while publishing queue state. */
    register void (*callback)(void) __asm__("$4");
    RrLibQueueRecord32 *p = StRingHeaders + StRingStartIndex;
    p->state = 2;
    StLastReadyLoc = p->word1C;
    value = p->value08;
    next = StRingWriteIndex;
    callback = StDataReadyFunc;
    StLastReadyFrame = value;
    {
        /* ASM_BARRIER: Preserve the assembler temporary for publishing the next index. */
        register u8 *base __asm__("$1");
        /* ASM_BARRIER: Read callback and next index before publishing state and materializing its address. */
        __asm__ volatile("" : : "r"(next), "r"(callback) : "$1", "memory");
        /* PAL mapping: base + 0x2F8 is &StRingStartIndex. */
        base = (u8 *)0x80110000U;
        /* ASM_BARRIER: Expose the index publication in the callback-test branch delay slot. */
        __asm__("" : "=r"(base) : "0"(base));
        *(s32 *)(base + 0x2F8) = next;
    }
    if (callback != 0) callback();
    StDataReadyPending = 0;
}
