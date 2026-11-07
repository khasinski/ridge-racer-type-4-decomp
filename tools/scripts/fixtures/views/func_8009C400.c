/* GCC_VERSION: 970318 */

#include "rr/commands.h"

s32 _cardCmd59Rx(s32 value) {
    s32 result;
    switch(CardRxCount) {
    case 3: CardBytesRemaining=value & 255; break;
    case 4: *(u32 *)CardCmdResultPtrA=value & 255; break;
    case 5: *(u32 *)CardCmdResultPtrA+=(u32)(value & 255)<<8; break;
    }
    result=0;
    return result;
}
