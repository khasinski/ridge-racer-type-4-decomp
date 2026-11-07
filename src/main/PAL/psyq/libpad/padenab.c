/* CC1_FLAGS: -mno-split-addresses */
#include "types.h"
#include "rr/channels.h"
#include "rr/serial.h"
#include "rr/interrupts.h"
#include "psyq/libpad/padenab.h"
#include "rr/spu_tick_control.h"

extern s32 ChangeClearRCnt(s32,s32);
extern s32 SysDeqIntRP(s32,void *);
extern s32 SysEnqIntRP(s32,void *);

s32 _padChkVsync(void);

int _padIntPad(void) {
    s32 first,condition,enabled;
    u32 shifted;
    RrSerialPortRegisters *port=PadSio0Base;
    if(port->control&2) {*(u16 *)&port->control=0;goto out;}
    condition=PadFirstChannel;
    /* ASM_BARRIER: Form the enable page after the serial-control gate. */
    __asm__("" : : : "$2");
    enabled=1;
    /* ASM_BARRIER: Materialize the enable value before the PAL page address. */
    __asm__("" : "+r"(enabled) : "r"(condition) : "$1");
    {
        /* ASM_BARRIER: Retain the actual PAL state page for the enable-store branch delay. */
        register s32 *page __asm__("$1")=(s32 *)0x800B0000U;
        /* ASM_BARRIER: Expose the enable store independently of its global-address macro. */
        __asm__("" : "+r"(page));
        page[-0x4330/4]=enabled;
    }
    if(condition) {
        s32 *counter=PadPortTimeoutCounts;
        s32 value=*counter;
        if(value<150)*counter=value+1;
    }
    if(!PadLastChannel) {
        s32 *counter=PadPortTimeoutCounts+1;
        s32 value=*counter;
        if(value<150)*counter=value+1;
    }
    if(!PadComActive)goto out;
    first=PadFirstChannel;
    /* Stage 15 times the index; quotient/remainder preserve native record sizes. */
    shifted=(u32)first<<4;
    if(PadLastChannel<first)goto out;
    PadRecvStage=0;
    PadActiveChannel=first;
    if(!_padInitSioMode((RrChannelRecordF0 *)((unsigned long)PadChannelRecords+(long)(s32)((shifted-(u32)first)*(sizeof(RrChannelRecordF0)/15)+(u32)first*(sizeof(RrChannelRecordF0)%15)))))PadAdvanceChannelFn(65535);
    PadActuatorPowerUsed=0;
    while(PadActiveChannel<=PadLastChannel)_padSioStageStep(PadChannelRecords+PadActiveChannel);
    PadSio0Base->baud=0x88;
out:
    return 0;
}

s32 _padChkVsync(void) {
    s32 previous = PadChkVsyncFlag;
    /* ASM_BARRIER: use the original assembler temporary for the store address. */
    register s32 *base __asm__("$1");
    /* ASM_BARRIER: load the prior value before materializing the store address. */
    __asm__ volatile("" : "=r"(previous) : "0"(previous) : "$1");
    /* PAL RAM mapping: PadChkVsyncFlag is base plus the signed offset below. */
    base = (s32 *)0x800B0000U;
    /* ASM_BARRIER: expose a register-relative store so it can occupy the return slot. */
    __asm__("" : "=r"(base) : "0"(base));
    base[(-0x4330) / 4] = 0;
    return previous;
}

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
