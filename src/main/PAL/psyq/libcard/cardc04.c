/* RODATA 0x80012158 0x20 */
/* GCC_VERSION: 970318 */
/* MASPSX_FLAGS: --assembler-profile=gnu */
#include "rr/commands.h"
#include "psyq/libcard/cardc07.h"
#include "psyq/libcard/cardc03.h"
#include "psyq/libcard/cardc04.h"

extern s32 _cardCheckAddress(s32, s32);

s32 _cardCmd5BBegin(s32 request, u8 *buffer);
int _cardCmd5BTx();
s32 _cardCmd5BRx(s32 value);
s32 _cardCmd5BRangeBegin(s32 request, u8 *buffer, s32 sector, s32 input_count);
int _cardCmd5BRangeTx();
s32 _cardCmd5BRangeRx(s32 value);
s32 _cardCmd5CRangeBegin(s32 request, u8 *buffer, s32 sector, s32 input_count);

s32 _cardCmd5BBegin(s32 request, u8 *buffer) {
    s32 result;
    s32 active;
    if (!rr_command_begin(request)) return 0;
    /* ASM_BARRIER: keep success initialization before the command configuration stores. */
    __asm__ volatile("" : "=r"(result) : "0"(1) : "$3", "memory");
    CardCurrentCommand = 3;
    CardCommandReader = _cardCmd5BTx;
    CardWriteHandler = _cardCmd5BRx;
    active = 2;
    CardCmdBuffer = buffer;
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

int _cardCmd5BTx() {
    register int r asm("$2");
    register int k asm("$2");
    int d = CardTxCount;
    k = 2;
    if (d == k) r = 0x5B;
    else r = 0;
    return r;
}

s32 _cardCmd5BRx(s32 value) {
    s32 phase=CardRxCount;
    /* ASM_BARRIER: Retain the copied input for status-byte publication. */
    register s32 saved __asm__("$5")=value;
    /* ASM_BARRIER: Share the zero result after all response stores. */
    register s32 result __asm__("$2");
    if(phase==5) {
        CardBytesRemaining=saved & 255;
        /* ASM_BARRIER: Keep the copied raw input distinct from its masked status. */
        __asm__ volatile("" : : "r"(saved));
    }
    else if(phase==10) CardCmdBuffer[4]=(u32)value-1;
    else if(phase>=6) CardCmdBuffer[phase-6]=value;
    result=0;
    /* ASM_BARRIER: Preserve the common return after response publication. */
    __asm__ volatile("" : : "r"(result));
    return result;
}

s32 _cardCmd5BRangeBegin(s32 request, u8 *buffer, s32 sector, s32 input_count) {
    s32 result;
    s32 active;
    /* ASM_BARRIER: retain the count in the original callee-saved register across validation calls. */
    register s32 count __asm__("$17") = input_count;
    /* ASM_BARRIER: finish preserving count before preparing the first validation call. */
    __asm__("" : "=r"(count) : "0"(count));
    if (!_cardCheckAddress(0, sector)) return 0;
    if (!_cardCheckAddress(0, (s32)((u32)sector + (u32)count - 1))) return 0;
    if (!rr_command_begin(request)) return 0;
    /* ASM_BARRIER: keep success initialization before the command configuration stores. */
    __asm__ volatile("" : "=r"(result) : "0"(1) : "$3", "memory");
    CardCurrentCommand = 4;
    CardCommandReader = _cardCmd5BRangeTx;
    CardWriteHandler = _cardCmd5BRangeRx;
    active = 2;
    CardCmdBuffer = buffer;
    CardCmdSector = sector;
    CardVarRxLength = count;
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

/* legacy typing of shared globals in this part of the unit */
#define CardCmdSector (*(unsigned char (*))&CardCmdSector)

int _cardCmd5BRangeTx() {
    switch (CardTxCount) {
    case 2: return 0x5B;
    case 3: return 1;
    case 5: return CardCmdSector;
    case 6: return D_80114725;
    case 7: return D_80114726;
    case 8: return D_80114727;
    case 9: return CardVarRxLength;
    }
    return 0;
}

#undef CardCmdSector

s32 _cardCmd5BRangeRx(s32 value) {
    s32 phase=CardRxCount;
    s32 result;
    if(phase==9) CardBytesRemaining=(u32)CardVarRxLength+1;
    else if(phase>=11) CardCmdBuffer[phase-11]=value;
    result=0;
    return result;
}

s32 _cardCmd5CRangeBegin(s32 request, u8 *buffer, s32 sector, s32 input_count) {
    s32 result;
    s32 active;
    /* ASM_BARRIER: retain the count in the original callee-saved register across validation calls. */
    register s32 count __asm__("$17") = input_count;
    /* ASM_BARRIER: finish preserving count before preparing the first validation call. */
    __asm__("" : "=r"(count) : "0"(count));
    if (!_cardCheckAddress(1, sector)) return 0;
    if (!_cardCheckAddress(1, (s32)((u32)sector + (u32)count - 1))) return 0;
    if (!rr_command_begin(request)) return 0;
    /* ASM_BARRIER: keep success initialization before the command configuration stores. */
    __asm__ volatile("" : "=r"(result) : "0"(1) : "$3", "memory");
    CardCurrentCommand = 5;
    CardCommandReader = _cardCmd5CRangeTx;
    CardWriteHandler = _cardCmd5CRangeRx;
    active = 2;
    CardCmdBuffer = buffer;
    CardCmdSector = sector;
    CardVarRxLength = count;
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
