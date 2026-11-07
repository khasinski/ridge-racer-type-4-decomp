/* GCC_VERSION: 2.7.2 */
#include "types.h"
extern s32 CardCommandStatus;
extern s32 CardTxCount;
extern s32 CardReceiveIndex;
extern s32 CardRxCount;
extern s32 CardBytesRemaining;
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
