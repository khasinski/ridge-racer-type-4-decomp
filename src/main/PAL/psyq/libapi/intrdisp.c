/* MASPSX_FLAGS: --assembler-profile=gnu */
/* CC1_FLAGS: -mno-split-addresses */
#include "rr/interrupt_dispatch.h"
#include "rr/library.h"
#include "psyq/libapi/intrdisp.h"
#include "rr/spu_tick_control.h"

extern s32 ChangeClearPAD(s32);
extern s32 ChangeClearRCnt(s32,s32);
extern void ResetEntryInt();
extern void HookEntryInt(void *data);

RrLibInterruptCallback setIntr(s32 id,RrLibInterruptCallback callback);
RrLibSavedState *stopIntr(void);
RrLibSavedState *restartIntr(void);
void memclr();

void trapIntr(void) {
    RrLibSavedState *state=&IntrEnv;
    RrLibInterruptCallback *base;
    register u32 one __asm__("$19");
    RrLibInterruptCallback *callbacks;
    u32 pending;
    u32 next;
    register s32 index __asm__("$17");
    if(state->active==0) {
        DebugPrintStub(UnexpectedInterruptFmt,*I_STAT);
        ReturnFromException();
    }
    {
        register volatile u16 *status __asm__("$4")=I_STAT;
        u32 enabled=state->enabled_mask;
        volatile u16 *mask;
        register u32 statusValue __asm__("$4");
        register u32 maskValue __asm__("$2");
        state->dispatching02=1;
        mask=I_MASK;
        statusValue=*status;
        maskValue=*mask;
        /* Preserve the initialized register values through mask composition. */
        __asm__("" : : "r"(enabled),"r"(statusValue));
        enabled&=statusValue;
        next=maskValue&enabled;
        pending=next;
    }
    if(next) {
        one=1;
        base=state->callbacks;
        do {
            index=0;
            if(pending) {
                callbacks=base;
again:
                if(index>=11)goto reread;
                if(pending&1) {
                    RrLibInterruptCallback callback;
                    RR_IRQ_ACK16(I_STAT,~(one<<index));
                    callback=*callbacks;
                    if(callback) callback();
                }
                callbacks++;
                pending>>=1;
                index++;
                if(pending&0xFFFF)goto again;
            }
reread:
            {
                register volatile u16 *status __asm__("$4")=I_STAT;
                u32 enabled=IntrEnv.enabled_mask;
                volatile u16 *mask=I_MASK;
                register u32 statusValue __asm__("$4")=*status;
                register u32 maskValue __asm__("$2")=*mask;
                __asm__("" : : "r"(enabled),"r"(statusValue));
                enabled&=statusValue;
                next=maskValue&enabled;
            }
            pending=next;
        } while(next);
    }
    {
        volatile u16 *status=I_STAT;
        volatile u16 *mask=I_MASK;
        if(*status & *mask) {
            s32 *counter=&IntrStuckCount;
            u32 count=(u32)*counter;
            s32 previous=(s32)count;
            count++;
            *counter=(s32)count;
            if(previous>=0x801) {
                DebugPrintStub(({const char *format=IntrTimeoutFmt; __asm__("" : "+r"(format));format;}),*status,*mask);
                IntrStuckCount=0;
                RR_IRQ_ACK16(I_STAT,0);
            }
        } else IntrStuckCount=0;
    }
    IntrEnv.dispatching02=0;
    ReturnFromException();
}

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

/* legacy typing of shared globals in this part of the unit */
#define I_MASK ((unsigned short *)I_MASK)
#define I_STAT ((unsigned short *)I_STAT)

RrLibSavedState *stopIntr(void) {
  RrLibSavedState *p = &IntrEnv;
  unsigned short *a, *c; int *b; int t;
  if (p->active == 0) return 0;
  EnterCriticalSection();
  a = I_MASK;
  b = DPCR_Intr;
  p->register16 = *a;
  t = *b;
  c = I_STAT;
  p->register32 = t;
  *a = 0;
  *c = *(volatile unsigned short *)a;
  *DPCR_Intr = *DPCR_Intr & 0x77777777;
  ResetEntryInt();
  p->active = 0;
  return p; }

#undef I_MASK
#undef I_STAT

/* legacy typing of shared globals in this part of the unit */
#define I_MASK ((u16 *)I_MASK)

RrLibSavedState *restartIntr(void) {
    RrLibSavedState *state = &IntrEnv;
    if (state->active != 0) return 0;
    HookEntryInt(state->callback_data);
    state->active = 1;
    *I_MASK = state->register16;
    {
        s32 *destination = DPCR_Intr;
        s32 value = state->register32;
        /* ASM_BARRIER: retain the original load-delay nop before the call/store pair. */
        __asm__ volatile("nop");
        *destination = value;
    }
    ExitCriticalSection();
    return state;
}

#undef I_MASK

void memclr(p, n) int *p; int n; {
    while (n--) *p++ = 0;
}
