/* GCC_VERSION: 970318 */
#include "rr/commands.h"
s32 _cardCmd5BRangeRx(s32 value) {
    s32 phase=CardRxCount;
    s32 result;
    if(phase==9) CardBytesRemaining=(u32)CardVarRxLength+1;
    else if(phase>=11) CardCmdBuffer[phase-11]=value;
    result=0;
    return result;
}
