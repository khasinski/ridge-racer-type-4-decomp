/* GCC_VERSION: 2.7.2 */

#include "rr/commands.h"

/* legacy typing of shared globals in this part of the unit */
#define CardCmdBuffer (*(int (*))&CardCmdBuffer)

#define CardCommandReader ((int *)CardCommandReader)

#define CardWriteHandler ((int *)CardWriteHandler)

#undef CardCmdBuffer

#undef CardCommandReader

#undef CardWriteHandler

/* legacy typing of shared globals in this part of the unit */
#define CardCmdValue (*(char (*))&CardCmdValue)

#define CardCmdBuffer ((char *)CardCmdBuffer)

#define CardCommandReader ((int *)CardCommandReader)

#define CardWriteHandler ((int *)CardWriteHandler)

#undef CardCmdValue

#undef CardCmdBuffer

#undef CardCommandReader

#undef CardWriteHandler

extern s32 _cardCmd58Tx(void);

extern s32 _cardCmd58Rx(s32);

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
