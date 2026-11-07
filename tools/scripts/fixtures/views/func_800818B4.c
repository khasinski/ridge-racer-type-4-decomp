/* CC1_FLAGS: -mno-split-addresses */

#include "rr/library.h"

#include "rr_asm.h"

typedef RrLibControllerCallback Control;

extern RrLibStateB0 *_ss_score[];

void _SsSetControlChange(s32 bank, s32 row, s32 control) {
/* ASM_BARRIER: Retain the original scratch or argument register through controller dispatch. */
register u32 bits __asm__("$2");
    /* ASM_BARRIER: Retain the original scratch or argument register through controller dispatch. */
    register s32 normalized_bank __asm__("$8");
    /* ASM_BARRIER: Retain the original scratch or argument register through controller dispatch. */
    register s32 normalized_row __asm__("$7");
    /* ASM_BARRIER: Retain the original scratch or argument register through controller dispatch. */
    register s32 argument_bank __asm__("$4");
    /* ASM_BARRIER: Retain the original scratch or argument register through controller dispatch. */
    register s32 argument_row __asm__("$5");
    s32 index, offset;
    s32 code;
    register s32 threshold;
    RrLibStateB0 *state;
    u8 *cursor;
    u32 value;
    Control callback;
    bits = (u32)bank<< 16;
    normalized_bank = (s32)bits>> 16;
    index = normalized_bank*(s32)sizeof(void *);
    /* ASM_BARRIER: Finish the bank index before normalizing the row. */
    __asm__("" : "=r"(row) : "0"(row), "r"(index));
    bits = (u32)row<< 16;
    normalized_row = (s32)bits>> 16;
    offset = normalized_row*(s32)sizeof(RrLibStateB0);
    /* ASM_BARRIER: Complete record offsets and canonical selectors before the indexed table load. */
    __asm__ volatile("" : : "r"(index), "r"(offset), "r"(normalized_bank), "r"(normalized_row) : "memory");
    state = _ss_score[normalized_bank]+normalized_row;
    cursor = state->c0;
    value = *cursor++;
    state->c0 = cursor;
    code = control & 255;
    if(code==64) goto c64;
    if(code<65) {
        if(code==7) goto c7;
        if(code<8) {
            if(code==0) goto c0;
            if(code==6) goto c6;
            goto fallback;
        }
        if(code==10) goto c10;
        if(code==11) goto c11;
        goto fallback;
    }
    if(code==99) goto c99;
    if(code<100) {
        if(code==91) goto c91;
        if(code==98) goto c98;
        goto fallback;
    }
    threshold = 101;
    /* ASM_BARRIER: Keep the equality constant opaque so the final signed comparison retains its immediate operand. */
    __asm__ volatile("" : "=r"(threshold) : "0"(threshold));
    if(code==threshold) goto c101;
    if(code<101) goto c100;
    if(code==121) goto c121;
    goto fallback;
c0:
        argument_bank = normalized_bank; argument_row = normalized_row;
        state->b26 = value; goto decode;
c6:
        argument_bank = normalized_bank;
        callback = SsCtrlHandlerCC6;
        argument_row = normalized_row;
        goto dispatch;
c7:
        argument_bank = normalized_bank;
        callback = SsCtrlHandlerCC7;
        argument_row = normalized_row;
        goto dispatch;
c10:
        argument_bank = normalized_bank;
        callback = SsCtrlHandlerCC10;
        argument_row = normalized_row;
        goto dispatch;
c11:
        argument_bank = normalized_bank;
        callback = SsCtrlHandlerCC11;
        argument_row = normalized_row;
        goto dispatch;
c64:
        argument_bank = normalized_bank;
        callback = SsCtrlHandlerCC64;
        argument_row = normalized_row;
        goto dispatch;
c91:
        argument_bank = normalized_bank;
        callback = SsCtrlHandlerCC91;
        argument_row = normalized_row;
        goto dispatch;
c98:
        argument_bank = normalized_bank;
        callback = SsCtrlHandlerCC98;
        argument_row = normalized_row;
        goto dispatch;
c99:
        argument_bank = normalized_bank;
        callback = SsCtrlHandlerCC99;
        argument_row = normalized_row;
        goto dispatch;
c100:
        argument_bank = normalized_bank;
        callback = SsCtrlHandlerCC100;
        argument_row = normalized_row;
        goto dispatch;
c101:
        argument_bank = normalized_bank;
        argument_row = normalized_row;
        callback = SsCtrlHandlerCC101;
        /* ASM_BARRIER: Pipeline annotation: fill the indirect-target load delay on the fallthrough dispatch path. */
        RR_NOP_DEP_ON(callback);
dispatch:
        callback(argument_bank, argument_row, value);
        return;
c121:
        SsCtrlHandlerCC121(normalized_bank, normalized_row);
        return;
fallback:
    argument_bank = (s16)bank;
    argument_row = (s16)row;
decode:
    state->w90 = _SsReadDeltaValue(argument_bank, argument_row);
}
