#include "rr/cd_read_stop.h"
#include "rr/cd.h"

extern RrLibCompletionCallback volatile CdReadSavedReadyCallback;
extern void CdDataSync(s32);
extern s32 VSync(s32);
extern RrLibCompletionCallback CdSyncCallback(RrLibCompletionCallback);
extern RrLibCompletionCallback CdReadyCallback(RrLibCompletionCallback);
extern RrLibCompletionCallback CdDataCallback(RrLibCompletionCallback);
extern void cd_read_restore_sync(void);

#define RR_READ_OWNER_OFFSET \
    (sizeof(RrLibReadRequestState)-sizeof(RrLibReadRestoreTail))
#define RR_PAIR_OWNER_OFFSET (RR_READ_OWNER_OFFSET+sizeof(RrLibCompletionCallback))
#define RR_PAIR_TAIL(pair) \
    ((volatile RrLibReadRestoreTail*)((u8*)(pair)-sizeof(RrLibCompletionCallback)))
#define RR_PAIR_REQUEST(pair) \
    ((volatile RrLibReadRequestState*)((u8*)(pair)-RR_PAIR_OWNER_OFFSET))
#define RR_PAIR_PENDING(pair) (RR_PAIR_REQUEST(pair)->finished24)

void CdReadBreak(void) {
    volatile RrLibReadRequestState *root;
    /* ASM_BARRIER: form the completion-publication address in its original temporary. */
    register RrLibReadRequestState *publish __asm__("$2");
    /* ASM_BARRIER: retain the callback slot while deriving its containing record. */
    register RrLibCompletionCallback *saved __asm__("$16");
    RrLibCompletionCallback restored;
    /* ASM_BARRIER: retain the callback-pair address through the stop sequence. */
    register RrLibCompletionCallback volatile *pair __asm__("$17");
    volatile s32 *pending_a,*pending_b;
    u32 first_tick,second_tick;
    s32 decision;
    if(CD_read_dma_mode&1)CdDataSync(0);
    root=&CdReadState;
    /* ASM_BARRIER: capture the complete state address before initial publication. */
    __asm__("" : "+r"(root));
    root->remaining14=0;
    if(root->finished24!=0)goto begin_first_wait;
    decision=1;
    goto after_first_wait;
first_timeout:
    saved=&CdReadSavedSyncCallback;
    /* ASM_BARRIER: materialize the saved-callback slot before its first load. */
    __asm__("" : "+r"(saved));
    restored=*saved;
    root=(RrLibReadRequestState*)((u8*)saved-RR_READ_OWNER_OFFSET);
    CdSyncCallback(restored);
    decision=-1;
    /* The field's object type is ordinary s32. This publication follows callback
     * restoration and retains the original branch-delay store. */
    ((RrLibReadRequestState*)root)->finished24=0;
    goto after_first_wait;
second_timeout:
    saved=&CdReadSavedSyncCallback;
    /* ASM_BARRIER: materialize the saved-callback slot before the second timeout load. */
    __asm__("" : "+r"(saved));
    restored=*saved;
    root=(RrLibReadRequestState*)((u8*)saved-RR_READ_OWNER_OFFSET);
    CdSyncCallback(restored);
    root->finished24=0;
    goto done;
begin_first_wait:
    first_tick=VSync(-1);
    if(root->finished24!=0) {
        pending_a=&root->finished24;
first_poll:
        if((u32)VSync(-1)-first_tick>=121U)goto first_timeout;
        if(*pending_a!=0)goto first_poll;
    }
    decision=0;
after_first_wait:
    if(decision!=0) {
        /* ASM_BARRIER: keep callback restoration after the completed-wait branch. */
        __asm__("" : );
        pair=&CdReadSavedReadyCallback;
        /* ASM_BARRIER: capture the complete callback-pair address before setters. */
        __asm__("" : "+r"(pair));
        CdReadyCallback(RR_PAIR_TAIL(pair)->saved2C);
        if(CD_read_dma_mode&1)CdDataCallback(RR_PAIR_TAIL(pair)->saved30);
        CdSyncCallback(cd_read_restore_sync);
        CdControlF(9,0);
        publish=(RrLibReadRequestState*)((u8*)pair-RR_PAIR_OWNER_OFFSET);
        ((volatile RrLibReadRequestState*)publish)->finished24=1;
        if(RR_PAIR_PENDING(pair)!=0) {
            second_tick=VSync(-1);
            if(RR_PAIR_PENDING(pair)!=0) {
                pending_b=&RR_PAIR_PENDING(pair);
second_poll:
                if((u32)VSync(-1)-second_tick>=121U)goto second_timeout;
                if(*pending_b!=0)goto second_poll;
            }
        }
    }
done:
    return;
}
