/* GCC_VERSION: 2.7.2 */
#include "rr/commands.h"

extern s32 _cardCmd5BVarTx(void);
extern s32 _cardCmd5BVarRx(s32);

s32 _cardCmd5BVarBegin(s32 request, s32 parameter, u8 *buffer, u8 *first) {
    s32 result;
    s32 active;
    if (!rr_command_begin(request)) return 0;
    /* ASM_BARRIER: keep success initialization before the command configuration stores. */
    __asm__ volatile("" : "=r"(result) : "0"(1) : "$3", "memory");
    CardCurrentCommand = 14;
    CardCommandReader = _cardCmd5BVarTx;
    CardWriteHandler = _cardCmd5BVarRx;
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
