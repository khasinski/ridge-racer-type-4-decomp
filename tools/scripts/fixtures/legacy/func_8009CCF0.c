/* GCC_VERSION: 970318 */
#include "rr_asm.h"
#include "rr/commands.h"
#include "rr/serial.h"
s32 _cardCmd5BVarRx(s32 value) {
    s32 saved=value;
    s32 received;
    RrSerialPortRegisters *port;
    s32 result;
    if(CardRxCount==3) {
        port=CardCmdSioRegs;
        RR_NOP_DEP_ON(port);
        while(!(port->status & 2)) { }
        D_80114728=CardCmdSioRegs->data;
    } else if(CardRxCount>=4) {
        if((u32)CardRxCount==(u32)D_80114728+4) {
            port=CardCmdSioRegs;
            RR_NOP_DEP_ON(port);
            while(!(port->status & 2)) { }
            received=CardCmdSioRegs->data;
            CardCmdBuffer=CardCmdResultPtrA;
            D_8011472C.value=1;
            CardBytesRemaining=received+1;
        }
        {
            s32 count=D_80114728;
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
