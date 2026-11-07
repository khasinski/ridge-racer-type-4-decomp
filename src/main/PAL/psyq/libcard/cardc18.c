/* GCC_VERSION: 2.7.2 */
#include "rr/commands.h"

s32 _cardSectorBegin(s32 a0, s32 a1, s32 a2) {
    int r;
    CardCmdSector = a1;
    if (((u32)a1 - 1) < 0xF) {
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
        if (r != 0) {
            D_8011472C.value = a2 != 0;
            CardCurrentCommand = 0xB;
            CardCommandReader = _cardSectorTx;
            CardWriteHandler = _cardSectorRx;
            CardRetriesLeft = 1;
            CardCommandStatus = 2;
            return 1;
        }
        return 0;
    }
    return 0;
}
