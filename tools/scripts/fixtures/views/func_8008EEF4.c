#include "rr/cd.h"

#include "rr/library.h"

#include "rr/cd_read_request.h"

#define RR_READ_OWNER_OFFSET \
    (sizeof(RrLibReadRequestState)-sizeof(RrLibReadRestoreTail))

#define RR_PAIR_OWNER_OFFSET (RR_READ_OWNER_OFFSET+sizeof(RrLibCompletionCallback))

#define RR_PAIR_TAIL(pair) \
    ((volatile RrLibReadRestoreTail*)((u8*)(pair)-sizeof(RrLibCompletionCallback)))

#define RR_PAIR_REQUEST(pair) \
    ((volatile RrLibReadRequestState*)((u8*)(pair)-RR_PAIR_OWNER_OFFSET))

#define RR_PAIR_PENDING(pair) (RR_PAIR_REQUEST(pair)->finished24)

/* legacy typing of shared globals in this part of the unit */
#define CdReadState (*(RrLibTransferState38 (*))&CdReadState)

#define CdReadyCallback ((void (*)(RrLibCompletionCallback))CdReadyCallback)

#define CdDataCallback ((void (*)(RrLibCompletionCallback))CdDataCallback)

#define CdSyncCallback ((void (*)(RrLibCompletionCallback))CdSyncCallback)

#define cd_read_restore_sync ((void (*)(void))cd_read_restore_sync)

#undef CdReadState

#undef CdReadyCallback

#undef CdDataCallback

#undef CdSyncCallback

#undef cd_read_restore_sync

/* legacy typing of shared globals in this part of the unit */
#define cd_read_restore_sync ((void (*)(void))cd_read_restore_sync)

#undef cd_read_restore_sync

/* legacy typing of shared globals in this part of the unit */
#define CdControlB ((s32 (*)(s32,const u8*,u8*))CdControlB)

#undef CdControlB

extern s32 VSync(s32);

extern RrLibCompletionCallback CdSyncCallback(RrLibCompletionCallback);

extern RrLibCompletionCallback CdReadyCallback(RrLibCompletionCallback);

extern RrLibCompletionCallback CdDataCallback(RrLibCompletionCallback);

extern u8 CdStatus(void);

s32 CdRead(s32 sectors,u8 *buffer,u32 mode) {
    s32 count=sectors;
    u8 *destination=buffer;
    /* ASM_BARRIER: preserve request mode across pending-completion helpers. */
    register u32 format __asm__("$18");
    u32 start;
    /* ASM_BARRIER: retain the complete request address across callback setters. */
    register RrLibReadRequestState *state __asm__("$16");
    volatile s32 *pending;
    /* ASM_BARRIER: use the original constructor argument register for case stores. */
    register RrLibReadRequestState *control __asm__("$4");
    u32 mode_stage=mode;
    s32 command,kind;
    RrLibCompletionCallback *restore_slot;
    RrLibCompletionCallback restored;
    u32 default_step;
    RrLibCompletionCallback zero;
    s32 result,busy;
    /* ASM_BARRIER: materialize the complete fallback address in its temporary. */
    register RrLibReadRequestState *fallback __asm__("$3");
    const u8 *parameters;
    pending=&CdReadStatePending;
    /* ASM_BARRIER: capture the pending flag's complete address before polling. */
    __asm__("" : "+r"(pending));
    busy=*pending;
    format=mode_stage;
    if(busy!=0) {
        start=VSync(-1);
        if(*pending!=0) {
wait:
            if((u32)VSync(-1)-start>=121U)goto timeout;
            if(*pending!=0)goto wait;
        }
    }
setup:
    /* ASM_BARRIER: keep the timeout jump aimed at the complete constructor entry. */
    __asm__("" : );
    control=&CdReadState;
    control->mode0C=format;
    kind=control->mode0C&0x30;
    switch(kind) {
/* The original mode-dispatch block also holds timeout recovery, which returns
 * to setup before selecting the transfer step. */
timeout:
        restore_slot=&CdReadSavedSyncCallback;
        /* ASM_BARRIER: capture the saved-callback slot before deriving its owner. */
        __asm__("" : "+r"(restore_slot));
        restored=*restore_slot;
        state=(RrLibReadRequestState*)((u8*)restore_slot-
              (sizeof(RrLibReadRequestState)-sizeof(RrLibReadRestoreTail)));
        CdSyncCallback(restored);
        ((volatile RrLibReadRequestState*)state)->finished24=0;
        goto setup;
    case 0:
        control->step10=0x200;
        break;
    case 32:
        control->step10=0x249;
        break;
    default:
        default_step=0x246;
        /* ASM_BARRIER: stage the fallback value before its address temporary. */
        __asm__("" : "+r"(default_step) : : "$3");
        fallback=&CdReadState;
        /* ASM_BARRIER: retain the complete fallback address for its field store. */
        __asm__("" : "+r"(fallback));
        fallback->step10=default_step;
        break;
    }
    state=&CdReadState;
    zero=0;
    state->mode0C|=0x20;
    /* ASM_BARRIER: publish mode before the request buffer and sector count. */
    __asm__("" : "+r"(state), "+r"(zero) : : "memory");
    state->buffer04=destination;
    state->sectors00=count;
    state->restore.saved28=CdSyncCallback(zero);
    state->restore.saved2C=CdReadyCallback(0);
    if(CD_read_dma_mode&1)state->restore.saved30=CdDataCallback(0);
    state->start1C=VSync(-1);
    if(CdStatus()&0xE0) {
        command=9;
        parameters=0;
        /* ASM_BARRIER: form the command and parameter arguments independently. */
        __asm__("" : "+r"(parameters) : "r"(command));
        CdControlB(command,parameters,0);
    }
    result=cd_read_retry(0)>0;
    /* ASM_BARRIER: finish the Boolean result before restoring saved temporaries. */
    __asm__("" : "+r"(result) : : "$17", "$19");
    return result;
}
