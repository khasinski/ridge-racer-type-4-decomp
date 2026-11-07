/* GCC_VERSION: 970318 */
#include "rr_asm.h"
#include "rr/commands.h"
#include "rr/serial.h"
#include "psyq/libcard/cardc03.h"
#include "psyq/libcard/cardc12.h"

s32 _cardCmd5DBegin(s32 request, s32 parameter, s32 count);
s32 _cardCmd5DTx(void);
s32 _cardCmd5DValueBegin(s32 request, s32 parameter);
s32 _cardCmd5EBegin(s32 request, s32 parameter, s32 count);
int _cardCmd5ETx();
s32 _cardCmd5FBegin(s32 request, s32 parameter);
int _cardCmd5FTx();

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

s32 _cardCmd5DBegin(s32 request, s32 parameter, s32 count) {
    s32 result;
    s32 active;
    if (!rr_command_begin(request)) return 0;
    /* ASM_BARRIER: keep success initialization before the command configuration stores. */
    __asm__ volatile("" : "=r"(result) : "0"(1) : "$3", "memory");
    CardCurrentCommand = 6;
    CardCommandReader = _cardCmd5DTx;
    CardWriteHandler = _cardRxStatusByte;
    active = 2;
    CardCmdSector = parameter;
    D_80114728 = count;
    D_8011472C.value = 0;
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

s32 _cardCmd5DTx(void) {
    s32 result;
    switch(CardTxCount) {
    case 2: result=0x5D; break;
    case 4: result=CardCmdSector; break;
    case 5: result=D_8011472C.value; break;
    case 6: result=D_80114728; break;
    default: result=0; break;
    }
    return result;
}

s32 _cardCmd5DValueBegin(s32 request, s32 parameter) {
    s32 result;
    s32 active;
    if (!rr_command_begin(request)) return 0;
    /* ASM_BARRIER: keep success initialization before the command configuration stores. */
    __asm__ volatile("" : "=r"(result) : "0"(1) : "$3", "memory");
    CardCurrentCommand = 7;
    CardCommandReader = _cardCmd5DTx;
    CardWriteHandler = _cardRxStatusByte;
    active = 2;
    D_8011472C.value = parameter;
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

s32 _cardCmd5EBegin(s32 request, s32 parameter, s32 count) {
    s32 result;
    s32 active;
    if (!rr_command_begin(request)) return 0;
    /* ASM_BARRIER: keep success initialization before the command configuration stores. */
    __asm__ volatile("" : "=r"(result) : "0"(1) : "$3", "memory");
    CardCurrentCommand = 13;
    CardCommandReader = _cardCmd5ETx;
    CardWriteHandler = _cardRxStatusByte;
    active = 2;
    CardCmdSector = parameter;
    D_80114728 = count;
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

int _cardCmd5ETx() {
    int v = CardTxCount;
    int r;
    if (v == 4) goto c4;
    if (v < 5) {
        if (v == 2) goto c2;
        r = 0;
        goto end;
    }
    if (v == 5) goto c5;
    r = 0;
    goto end;
c2:
    r = 0x5E;
    goto end;
c4:
    r = CardCmdSector;
    goto end;
c5:
    r = D_80114728;
end:
    return r;
}

s32 _cardCmd5FBegin(s32 request, s32 parameter) {
    s32 result;
    s32 active;
    if (!rr_command_begin(request)) return 0;
    /* ASM_BARRIER: keep success initialization before the command configuration stores. */
    __asm__ volatile("" : "=r"(result) : "0"(1) : "$3", "memory");
    CardCurrentCommand = 8;
    CardCommandReader = _cardCmd5FTx;
    CardWriteHandler = _cardRxStatusByte;
    active = 2;
    CardCmdSector = parameter;
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

int _cardCmd5FTx() {
    switch (CardTxCount) {
    case 2: return 0x5F;
    case 4: return CardCmdSector;
    }
    return 0;
}
