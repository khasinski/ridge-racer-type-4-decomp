/* GCC_VERSION: 970318 */
#include "rr/commands.h"
s32 _cardCmd5DTx(void) {
    s32 result;
    switch(CardTxCount) {
    case 2: result=0x5D; break;
    case 4: result=CardCmdSector; break;
    case 5: result=D_8011472C.value; break;
    case 6: result=D_80114728; break;
    default: result=0; break;
    }
    return result;
}
