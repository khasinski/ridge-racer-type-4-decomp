/* CC1_FLAGS: -mno-split-addresses */
#include "rr/channels.h"
#include "rr/channel_init.h"

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

s32 _padChannelDispatch(void *a0) {
    RrChannelRecordF0 *state=a0;
    /* ASM_BARRIER: Separate the saved channel from the early-call argument. */
    __asm__("" : "+r"(state));
    if(*state->status.packet==0xF3) {
        if(!state->responseE8)goto Y;
        if(state->status.phase==0xFF)goto zero;
        if(state->status.state49==2)PadResetChannelFn(state);
    }
    switch(state->status.phase) {
    case 1:
        _padCmdParaMode(state,1);
        return 0;
    case 0xFE:goto zero;
    case 0:case 0xFF:return 0;
    default:goto other;
    }
zero:
    a0=state;
Y:
    _padCmdParaMode(a0,0);
    return 0;
other:
    if(!state->status.update14)goto fallback;
    state->status.update14(state);
    goto out;
fallback:
    _padSendAtLoadInfo((unsigned char *)state);
out:
    return 0;
}
