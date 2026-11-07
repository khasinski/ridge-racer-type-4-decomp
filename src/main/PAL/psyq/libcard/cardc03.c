/* GCC_VERSION: 2.7.2 */
#include "psyq/libcard/cardc03.h"
extern volatile int CardTxCount;
extern unsigned char CardCmdSector;
extern unsigned char *CardCmdBuffer;
extern int CardRxCount;
extern int CardBytesRemaining;

int _cardCmd59ParamTx();
int _cardRxStatusByte();

int _cardCmd59ParamTx() {
    switch (CardTxCount) {
    case 2: return 0x59;
    case 4: return CardCmdSectorByte1;
    case 5: return CardCmdSector;
    }
    if (CardTxCount < 6) return 0;
    if (CardTxCount >= 10) return 0;
    return CardCmdBuffer[CardTxCount - 6];
}

int _cardRxStatusByte(a0) int a0; { if (CardRxCount == 3) CardBytesRemaining = a0 & 0xFF; return 0; }
