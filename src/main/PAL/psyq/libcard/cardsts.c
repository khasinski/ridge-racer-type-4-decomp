/* GCC_VERSION: 2.7.2 */
#include "types.h"
#include "psyq/libcard/cardsts.h"
#include "rr/commands.h"

extern volatile int CardCommandStatus;
extern unsigned char D_80114741;
extern int CardCurrentCommand;


int MemCardSync(a0, a1, a2) int a0; int *a1; int *a2; {
    if (CardCurrentCommand == 0 || CardCommandStatus == 1) {
        *a2 = 0;
        *a1 = 0;
        return -1;
    }
    if (a0 == 0) {
        while (CardCommandStatus < 5);
    }
    *a1 = CardCurrentCommand;
    switch (CardCommandStatus) {
    case 4:
    case 2:
        *a2 = 0;
        return 0;
    case 17:
        if (CardTxCount == 1) {
            *a2 = CardTxCount;
        } else {
    case 33:
            *a2 = 2;
        }
        goto fin;
    default:
        if (D_80114741 == 8) {
            *a2 = 3;
        } else {
            *a2 = 0;
        }
    fin:
        CardCurrentCommand = 0;
        return 1;
    }
}

/* legacy typing of shared globals in this part of the unit */
#define CardCommandStatus (*(s32 (*))&CardCommandStatus)

s32 _cardCmdReset(s32 value) {
    s32 result;
    if (!(CardCommandStatus & 1)) return 0;
    CardReceiveIndex = 0;
    CardRxCount = 0;
    CardBytesRemaining = 0;
    CardTxCount = 0;
    {
        /* ASM_BARRIER: retain the original assembler temporary for the final state store. */
        register s32 *base __asm__("$1");
        result = 1;
        /* ASM_BARRIER: preserve the shared return path and preceding stores. */
        __asm__("" : "=r"(result) : "0"(result) : "$1", "memory");
        /* PAL RAM mapping: CardCommandRequest is base minus 0x4214. */
        base = (s32 *)0x800B0000U;
        /* ASM_BARRIER: expose the register-relative store in the branch delay slot. */
        __asm__("" : "=r"(base) : "0"(base));
        base[-0x4214 / 4] = value;
    }
    return result;
}

#undef CardCommandStatus
