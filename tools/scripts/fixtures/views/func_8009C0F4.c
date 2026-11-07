/* CC1_FLAGS: -mno-split-addresses */

#include "rr/commands.h"

#include "rr/serial.h"

#include "rr_asm.h"

s32 _cardCmdStep(void) {
    register s32 initial=CardTxCount;
    s32 one;
    /* ASM_BARRIER: Keep each common-return result in the ABI result register. */
    register s32 answer __asm__("$2");
    /* ASM_BARRIER: Preserve the original masked exchange argument and delay slot. */
    register s32 byte __asm__("$4");
    register s32 value;
    one=1;
    if(initial==one) {
        s32 slot=CardCommandRequest;
        register u32 header;
        register u32 count;
        CardCommandStatus=4;
        /* ASM_BARRIER: Capture the request before preparing the state-four temporary. */
        __asm__("" : : : "$2");
        _cardSelectSlot(slot>>4);
        _cardXferByte(~((CardCommandRequest&15)+0x81));
        /* ASM_BARRIER: Retain an immediate increment instead of reusing saved constant one. */
        __asm__ __volatile__("" : "+r"(one));
        header=*(u8 *)&CardCommandRequest;
        count=CardReceiveIndex;
        CardTxTrace[0]=(header&15)-0x7F;
        CardReceiveIndex=count+1;
        answer=0;goto done;
    }
    {
        register u32 remaining=CardBytesRemaining;
        register RrCommandReadCallback reader=CardCommandReader;
        /* ASM_BARRIER: Publish the real remaining-count store in the reader call delay slot. */
        register s32 *page __asm__("$1");
        remaining--;
        /* ASM_BARRIER: Finish the decrement before forming its mapped store address. */
        __asm__("" : : : "$1");
        page=(s32 *)0x80110000U;
        /* ASM_BARRIER: Expose the PAL RAM page to the stock delay-slot scheduler. */
        __asm__("" : "+r"(page));
        page[0x4730/4]=remaining;
        byte=reader()&255;
    }
    {
        register s32 *count=&CardReceiveIndex;
        u32 index;
        index=*count;
        CardTxTrace[index]=byte;
        *count=index+1;
        value=_cardXferByte(byte);
    }
    byte=value&255;
    if(value<0) {answer=(u32)value-1000;goto done;}
    {
        register s32 *phase=&CardRxCount;
        u32 index;
        register RrCommandWriteCallback writer;
        index=*phase;
        CardRxTrace[index]=value;
        writer=CardWriteHandler;
        *phase=index+1;
        value=writer(byte);
    }
    if(value) {answer=(u32)value-2000;goto done;}
    if(CardBytesRemaining!=one) {answer=0;goto done;}
    if(CardCurrentCommand==14) goto special;
    /* ASM_BARRIER: Retain the separate fresh subtype read for the second comparison. */
    __asm__("" : : : "$2", "$3", "memory");
    if(CardCurrentCommand!=15) goto wait;
special:
    if((u32)CardRxCount==(u32)CardVarRxLength+4) goto final_byte;
wait:
    {
        RrSerialPortRegisters *port=CardCmdSioRegs;
        /* ASM_BARRIER: Keep the captured-port load gap before the shared polling label. */
        RR_NOP_DEP_ON(port);
        while(!(port->status&2)) {}
    }
final_byte:
    {
        register s32 *phase=&CardRxCount;
        u32 tick;
        u32 remaining,index;
        register u32 raw;
        RrSerialPortRegisters *port;
        register RrCommandWriteCallback writer;
        tick=CardTxCount;
        remaining=CardBytesRemaining;
        CardTxCount=tick+1;
        port=CardCmdSioRegs;
        CardBytesRemaining=remaining-1;
        raw=port->data;
        /* ASM_BARRIER: Read the device byte before loading the fresh receive index. */
        __asm__("" : "+r"(raw) : : "$3", "memory");
        index=*phase;
        value=raw&255;
        CardRxTrace[index]=value;
        index++;
        /* ASM_BARRIER: Increment the trace index before preparing the fresh writer and argument. */
        __asm__("" : "+r"(index) : : "$2", "$4");
        writer=CardWriteHandler;
        *phase=index;
        value=writer(value);
    }
    if(value) answer=(u32)value-3000;
    else answer=1;
done:
    return answer;
}
