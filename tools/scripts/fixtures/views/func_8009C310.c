/* GCC_VERSION: 2.7.2 */

#include "rr/commands.h"

extern s32 _cardCmd59Tx(void), _cardCmd59Rx(s32);

s32 _cardCmd59Begin(s32 a0, u32 *a1) {
    int r;
    if (!(CardCommandStatus & 1)) {
        r = 0;
    } else {
        CardReceiveIndex = 0; CardRxCount = 0; CardBytesRemaining = 0; CardTxCount = 0; CardCommandRequest = a0;
        r = 1;
    }
    if (!r) return 0;
    CardCmdSector = -2;
    CardCurrentCommand = 1;
    CardCommandReader = _cardCmd59Tx;
    CardWriteHandler = _cardCmd59Rx;
    CardCmdResultPtrA = a1;
    CardCommandStatus = 2;
    return 1;
}
