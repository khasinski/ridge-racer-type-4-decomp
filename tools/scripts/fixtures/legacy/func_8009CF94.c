/* GCC_VERSION: 970318 */
#include "rr_asm.h"
#include "rr/commands.h"
#include "rr/serial.h"
s32 _cardCmd5CVarRx(s32 unused) {
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
    }
    result=0;
    return result;
}
