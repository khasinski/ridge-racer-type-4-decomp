/* GCC_VERSION: 2.7.2 */
#include "rr/commands.h"
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
