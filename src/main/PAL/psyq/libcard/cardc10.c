/* GCC_VERSION: 970318 */
#include "rr_asm.h"
#include "rr/commands.h"
#include "rr/serial.h"
#include "psyq/libcard/cardc11.h"
#include "psyq/libcard/cardc12.h"
#include "psyq/libcard/cardc10.h"

s32 _cardCmd5CVarBegin(s32 request, s32 parameter, u8 *buffer, u8 *first);

s32 _cardCmd5BVarRx(s32 value) {
    s32 saved=value;
    s32 received;
    RrSerialPortRegisters *port;
    s32 result;
    if(CardRxCount==3) {
        port=CardCmdSioRegs;
        RR_NOP_DEP_ON(port);
        while(!(port->status & 2)) { }
        CardVarRxLength=CardCmdSioRegs->data;
    } else if(CardRxCount>=4) {
        if((u32)CardRxCount==(u32)CardVarRxLength+4) {
            port=CardCmdSioRegs;
            RR_NOP_DEP_ON(port);
            while(!(port->status & 2)) { }
            received=CardCmdSioRegs->data;
            CardCmdBuffer=CardCmdResultPtrA;
            CardCmdValue.value=1;
            CardBytesRemaining=received+1;
        }
        {
            s32 count=CardVarRxLength;
            s32 phase=CardRxCount;
            if((s32)((u32)count+5)<phase) {
                /* Preserve native pointer width and the original wrapping 32-bit difference. */
                unsigned long address=(s32)((u32)phase-(u32)count);
                address+=(unsigned long)CardCmdBuffer;
                ((u8 *)address)[-6]=saved;
            }
        }
    }
    result=0;
    return result;
}

s32 _cardCmd5CVarBegin(s32 request, s32 parameter, u8 *buffer, u8 *first) {
    s32 result;
    s32 active;
    if (!rr_command_begin(request)) return 0;
    /* ASM_BARRIER: keep success initialization before the command configuration stores. */
    __asm__ volatile("" : "=r"(result) : "0"(1) : "$3", "memory");
    CardCurrentCommand = 15;
    CardCommandReader = _cardCmd5CVarTx;
    CardWriteHandler = _cardCmd5CVarRx;
    active = 2;
    CardCmdSector = parameter;
    CardCmdBuffer = buffer;
    CardCmdResultPtrA = first;
    CardVarRxLength = 0;
    CardCmdValue.value = 0;
    {
        /* ASM_BARRIER: preserve the original assembler temporary for the mapped store. */
        register s32 *base __asm__("$1");
        /* ASM_BARRIER: finish parameter publication before materializing the state address. */
        __asm__ volatile("" : : : "$1", "memory");
        /* PAL RAM mapping: base[-0x421C / 4] is CardCommandStatus. */
        base = (s32 *)0x800B0000U;
        /* ASM_BARRIER: expose the active-state store in the branch delay slot. */
        __asm__("" : "=r"(base) : "0"(base));
        base[-0x421C / 4] = active;
    }
    return result;
}
