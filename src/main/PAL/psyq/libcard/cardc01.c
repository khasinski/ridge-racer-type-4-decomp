/* GCC_VERSION: 2.7.2 */
#include "rr/commands.h"
#include "psyq/libcard/cardc02.h"

extern s32 _cardCmd59Tx(void);

s32 _cardCmd59Begin(s32 a0, u32 *a1);
int _cardCmd59Tx();

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

int _cardCmd59Tx() {
    int a; 
    register int r __asm__("$2");
    int t; 
    register int c; 
    a = CardTxCount;
    t = 2;
    if (a == t) {
        r = 0x59;
        goto out;
    }
    r = 0;
    if (a < 2) goto out;
    if (a >= 6) goto out;
    c = a < 4;
    if (c) goto out;
    /* ASM_BARRIER: stops 2.7.2 if-converting the last compare into xori/negu/andi */
    __asm__("" : "=r"(r) : "0"(0xFF));
out:
    return r;
}
