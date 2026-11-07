/* CC1_FLAGS: -mno-split-addresses */

#include "rr/channels.h"

#include "rr/serial.h"

s32 _padIntPad(void) {
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
