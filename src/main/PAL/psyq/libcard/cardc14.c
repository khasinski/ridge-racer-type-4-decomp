/* GCC_VERSION: 2.7.2 */
#include "rr/commands.h"
#include "psyq/libcard/cardc07.h"
#include "psyq/libcard/cardc161.h"
#include "psyq/libcard/cardc162.h"
#include "psyq/libcard/cardc04.h"

int _cardCmd5BFixedBegin();
int _cardCmd5CFixedBegin();
s32 _cardCmd58Begin(s32 request, u32 *first, u32 *second);

/* legacy typing of shared globals in this part of the unit */
#define CardCmdBuffer (*(int (*))&CardCmdBuffer)
#define CardCommandReader ((int *)CardCommandReader)
#define CardWriteHandler ((int *)CardWriteHandler)

int _cardCmd5BFixedBegin(a0,a1) int a0; int a1; {
    int r;
    if (!(CardCommandStatus & 1)) {
        r = 0;
    } else {
        r = 1;
        CardReceiveIndex = 0;
        CardRxCount = 0;
        CardBytesRemaining = 0;
        CardTxCount = 0;
        CardCommandRequest = a0;
    }
    if (r == 0) return 0;
    CardCmdSector = 0x6000300;
    D_80114728 = 4;
    CardCurrentCommand = 0xA;
    CardCommandReader = (int *)_cardCmd5BRangeTx;
    CardWriteHandler = (int *)_cardCmd5BRangeRx;
    CardCmdBuffer = a1;
    CardCommandStatus = 2;
    return 1;
}

#undef CardCmdBuffer
#undef CardCommandReader
#undef CardWriteHandler

/* legacy typing of shared globals in this part of the unit */
#define D_8011472C (*(char (*))&D_8011472C)
#define CardCmdBuffer ((char *)CardCmdBuffer)
#define CardCommandReader ((int *)CardCommandReader)
#define CardWriteHandler ((int *)CardWriteHandler)

int _cardCmd5CFixedBegin(a0, a1) int a0; int a1; {
    int r;
    char *p;
    if (!(CardCommandStatus & 1)) {
        r = 0;
    } else {
        r = 1;
        CardReceiveIndex = 0;
        CardRxCount = 0;
        CardBytesRemaining = 0;
        CardTxCount = 0;
        CardCommandRequest = a0;
    }
    if (r == 0) return 0;
    if (a1 != 0) {
        CardCmdSector = 0xD800008;
    } else {
        CardCmdSector = 0xD800004;
    }
    p = &D_8011472C;
    CardCmdBuffer = p;
    *p = 2;
    D_80114728 = 1;
    CardCurrentCommand = 9;
    CardCommandReader = (int *)_cardCmd5CRangeTx;
    CardWriteHandler = (int *)_cardCmd5CRangeRx;
    CardCommandStatus = 2;
    return 1;
}

#undef D_8011472C
#undef CardCmdBuffer
#undef CardCommandReader
#undef CardWriteHandler

s32 _cardCmd58Begin(s32 request, u32 *first, u32 *second) {
    s32 result;
    s32 active;
    if (!rr_command_begin(request)) return 0;
    /* ASM_BARRIER: keep success initialization before the command configuration stores. */
    __asm__ volatile("" : "=r"(result) : "0"(1) : "$3", "memory");
    CardCurrentCommand = 100;
    CardCommandReader = _cardCmd58Tx;
    CardWriteHandler = _cardCmd58Rx;
    active = 2;
    CardCmdResultPtrA = first;
    CardCmdResultPtrB = second;
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
