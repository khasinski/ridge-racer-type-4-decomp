/* GCC_VERSION: 2.7.2 */
#include "rr/commands.h"

extern s32 _cardCmd5DTx(void);
extern s32 _cardRxStatusByte(s32);

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
    CardVarRxLength = count;
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

