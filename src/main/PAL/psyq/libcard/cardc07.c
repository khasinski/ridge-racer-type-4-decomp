/* RODATA 0x80012178 0x24 */
/* GCC_VERSION: 2.7.2 */
#include "psyq/libcard/cardc03.h"
#include "psyq/libcard/cardc04.h"
#include "psyq/libcard/cardc07.h"
extern volatile int CardTxCount;
extern int CardCmdBuffer;
extern int CardVarRxLength;
extern unsigned char CardCmdSector;
extern int CardRxCount;
extern int CardBytesRemaining;

int _cardCmd5CRangeTx() {
    switch (CardTxCount) {
    case 2: return 0x5C;
    case 3: return 1;
    case 5: return CardCmdSector;
    case 6: return CardCmdSectorByte1;
    case 7: return CardCmdSectorByte2;
    case 8: return CardCmdSectorByte3;
    case 9: return CardVarRxLength;
    case 4:
    case 10: break;
    default: return *(unsigned char *)(CardTxCount + CardCmdBuffer - 0xB);
    }
    return 0;
}

int _cardCmd5CRangeRx() { if (CardRxCount == 9) CardBytesRemaining = CardVarRxLength + 1; return 0; }
