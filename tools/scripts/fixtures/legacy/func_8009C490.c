/* GCC_VERSION: 2.7.2 */
#include "rr/commands.h"
extern s32 _cardCmd59ParamTx(void), _cardRxStatusByte(s32);

s32 _cardCmd59ParamBegin(s32 request, s32 parameter, u8 *buffer) {
    s32 mode;
    if (parameter >= 0) {
        if (parameter >= 16) return 0;
        {
            /* ASM_BARRIER: Use the original assembler temporary for parameter publication. */
            register s32 *base __asm__("$1");
            /* ASM_BARRIER: Keep parameter validation before materializing its PAL address. */
            __asm__ volatile("" : : : "$1", "memory");
            /* PAL mapping: base[0x4724 / 4] is CardCmdSector. */
            base = (s32 *)0x80110000U;
            /* ASM_BARRIER: Expose the PAL parameter store in the branch delay slot. */
            __asm__("" : "=r"(base) : "0"(base));
            base[0x4724 / 4] = parameter;
        }
    } else CardCmdSector = -1;
    if (!rr_command_begin(request)) return 0;
    mode = 2;
    /* ASM_BARRIER: Form command state 2 before callback materialization and parameter stores. */
    __asm__ volatile("" : "=r"(mode) : "0"(mode) : "$4", "memory");
    CardCommandReader = _cardCmd59ParamTx;
    CardCmdValue.buffer = buffer;
    CardCmdBuffer = buffer;
    CardCurrentCommand = mode;
    CardWriteHandler = _cardRxStatusByte;
    CardCommandStatus = mode;
    return 1;
}
