#include "rr/sequence_queue.h"

void *SoundQueueEnqueue(s32 id,s32 restart) {
    s32 n=0,used=0,copied;
    s16 *input,*scan;
    u8 *output;
    /* ASM_BARRIER: keep the final address in the ABI return register. */
    register u8 *result __asm__("$2");
    RrSequenceQueueState *state;
    RrSequenceQueueState *last;
    /* ASM_BARRIER: address temporary dies before each copy/return value in v0. */
    register u8 *copybase __asm__("$2");
    if(SoundSequenceSet==0)input=SoundSequenceTable[id][0];
    else input=SoundSequenceTable[id][1];
    if(input==(s16*)-1)return (void*)-1;
    state=&SoundState;
    if(state->active19D0==0) {
        state->active19D0=1;
        goto reset;
    }
    if(restart) {
reset:
        state->timer19D8=0;state->timer19DC=0;state->mode19E0=restart;
        state->x19E4=80;state->y19E6=80;
    }else{
        used=state->count19D4;
        if(*input>0) {
            scan=input;
            do {scan++;n++;}while(*scan>0);
        }
        if(used+n>=8)return (void*)-1;
    }
    copied=0;
    if(*input>0) {
        copybase=(u8*)&SoundState;
        output=(u8*)((unsigned long)(used*2)+(unsigned long)copybase);
        do{
            u16 value=*input++;
            copied++;used++;
            *(u16*)(output+0x19E8)=value;
            output+=2;
        }while(*input>0);
    }
    copybase=(u8*)0x800F0000U;
    /* ASM_BARRIER: preserve the PAL page for the final state-base displacement. */
    __asm__("" : "+r"(copybase));
    last=(RrSequenceQueueState*)(copybase+0x4DB8);
    /* Integer byte addressing retains full native pointers and target operand order. */
    result=(u8*)((unsigned long)(used*2)+(unsigned long)last);
    *(s16*)(result+0x19E8)=-1;
    last->count19D4=used;
    if(copied==0)last->active19D0=0;
    return result;
}
