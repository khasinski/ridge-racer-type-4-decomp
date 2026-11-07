/* CC1_FLAGS: -mno-split-addresses */

#include "rr/channels.h"

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
