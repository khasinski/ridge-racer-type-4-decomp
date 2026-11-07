/* MASPSX_FLAGS: --assembler-profile=gnu */

/* CC1_FLAGS: -mno-split-addresses */

#include "rr/interrupt_dispatch.h"

#include "rr/library.h"

/* legacy typing of shared globals in this part of the unit */
#define I_MASK ((unsigned short *)I_MASK)

#define I_STAT ((unsigned short *)I_STAT)

#undef I_MASK

#undef I_STAT

/* legacy typing of shared globals in this part of the unit */
#define I_MASK ((u16 *)I_MASK)

#undef I_MASK

extern s32 ChangeClearPAD(s32),ChangeClearRCnt(s32,s32);

RrLibInterruptCallback setIntr(s32 id,RrLibInterruptCallback callback) {
    s32 selector=id;
    /* ASM_BARRIER: Preserve the requested callback across kernel calls. */
    register RrLibInterruptCallback requested __asm__("$18")=callback;
    RrLibInterruptCallback *base;
    RrLibInterruptCallback *entry;
    RrLibInterruptCallback previous;
    RrLibSavedState *state;
    u32 saved;
    s32 disabled;
    /* ASM_BARRIER: Capture both full arguments before reusing argument registers for addresses. */
    __asm__("" : "+r"(selector), "+r"(requested));
    base=IntrEnv.callbacks;
    entry=(RrLibInterruptCallback *)((unsigned long)((u32)selector*sizeof(*base))+(unsigned long)base);
    previous=*entry;
    if(requested==previous) goto out;
    state=(RrLibSavedState *)((unsigned long)base-RR_FIELD_OFFSET(RrLibSavedState,callbacks));
    if(!state->active) goto out;
    {
        u16 raw=*I_MASK;
        *I_MASK=0;
        saved=raw;
    }
    if(requested) {
        u32 bit=1U<<selector;
        *entry=requested;
        saved |= bit;
        state->enabled_mask |= bit;
    } else {
        u32 bit;
        /* ASM_BARRIER: Keep the null callback's table store independent of its saved register. */
        __asm__("" : "+r"(requested));
        bit=~(1U<<selector);
        *entry=0;
        saved &= bit;
        *(u16 *)((unsigned long)base+RR_FIELD_OFFSET(RrLibSavedState,enabled_mask)-RR_FIELD_OFFSET(RrLibSavedState,callbacks)) &= bit;
    }
    if(selector==0) {
        disabled=requested==0;
        ChangeClearPAD(disabled);
        ChangeClearRCnt(3,disabled);
    }
    if(selector==4)ChangeClearRCnt(0,requested==0);
    if(selector==5)ChangeClearRCnt(1,requested==0);
    if(selector==6)ChangeClearRCnt(2,requested==0);
    *I_MASK=saved;
out:
    return previous;
}
