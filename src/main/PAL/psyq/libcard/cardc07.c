/* RODATA 0x80012178 0x24 */
/* GCC_VERSION: 2.7.2 */
#include "psyq/libcard/cardc03.h"
#include "psyq/libcard/cardc04.h"
#include "psyq/libcard/cardc07.h"
extern volatile int CardTxCount;
extern int CardCmdBuffer;
extern int D_80114728;
extern unsigned char CardCmdSector;
extern int CardRxCount;
extern int CardBytesRemaining;

int _cardCmd5CRangeTx() {
    switch (CardTxCount) {
    case 2: return 0x5C;
    case 3: return 1;
    case 5: return CardCmdSector;
    case 6: return D_80114725;
    case 7: return D_80114726;
    case 8: return D_80114727;
    case 9: return D_80114728;
    case 4:
    case 10: break;
    default: return *(unsigned char *)(CardTxCount + CardCmdBuffer - 0xB);
    }
    return 0;
}

int _cardCmd5CRangeRx() { if (CardRxCount == 9) CardBytesRemaining = D_80114728 + 1; return 0; }
