/* CC1_FLAGS: -mno-split-addresses */

#include "rr/channels.h"

void _padInitDirSeq(void) {
    /* ASM_BARRIER: reuse v0 for each callback address in source order. */
    register RrChannelUpdateCallback update __asm__("$2");
    /* ASM_BARRIER: preserve the original assembler temporary for the final slot. */
    register u8 *base __asm__("$1");
    PadChannelDispatchPtr = _padChannelDispatch;
    PadChannelBusyFn = _padChannelBusy;
    /* ASM_BARRIER: finish the first two callback stores before loading the third. */
    __asm__ volatile("" : : : "$2", "memory");
    update = _padPacketDone;
    /* PAL RAM mapping: PadPacketDoneCallback is base minus 0x4378. */
    base = (u8 *)0x800B0000U;
    /* ASM_BARRIER: expose the register-relative store in the return delay slot. */
    __asm__("" : "=r"(base) : "0"(base));
    *(RrChannelUpdateCallback *)(base - 0x4378) = update;
}
