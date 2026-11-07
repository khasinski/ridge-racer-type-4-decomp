/* CC1_FLAGS: -mno-split-addresses */

#include "rr/channels.h"

#include "rr/serial.h"

#include "rr/interrupts.h"

extern u32 PadIntQueueEntry[4];

extern s32 EnterCriticalSection(void),ChangeClearRCnt(s32,s32);

extern void ExitCriticalSection(void);

extern s32 SysDeqIntRP(s32,void *),SysEnqIntRP(s32,void *);

void _padStartCom(void) {
    RrInterruptRegisters *p;
    u32 mask;
    s32 *counters;
    {
        /* ASM_BARRIER: Keep the initial enable-state address in the assembler temporary. */
        register s32 *page __asm__("$1");
        /* ASM_BARRIER: Expose the initial state store in the critical-section call's delay slot. */
        __asm__ volatile("" : : : "$1", "memory");
        page=(s32 *)0x800B0000U;
        /* ASM_BARRIER: Preserve the PAL page-relative enable-state store. */
        __asm__("" : "+r"(page));
        page[-0x4360/4]=0;
    }
    EnterCriticalSection();
    SysDeqIntRP(2,PadIntQueueEntry);
    SysEnqIntRP(2,PadIntQueueEntry);
    p=I_STAT_Pad;
    p->pending=0xFFFFFFFEU;
    mask=p->mask;
    /* Ordinary final write view permits the original call-delay store. */
    *(u32 *)((u8 *)p+RR_FIELD_OFFSET(RrInterruptRegisters,mask))=mask|1;
    ChangeClearRCnt(3,0);
    ExitCriticalSection();
    PadResetChannelFn(PadChannelRecords);
    PadResetChannelFn(PadChannelRecords+1);
    counters=PadPortTimeoutCounts;
    counters[1]=0;counters[0]=0;
    PadComActive=1;
    /* ASM_BARRIER: Restore saved registers only after publishing the enabled state. */
    __asm__ volatile("" : : : "$16", "$31", "memory");
}
