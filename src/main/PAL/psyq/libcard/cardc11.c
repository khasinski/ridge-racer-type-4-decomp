/* GCC_VERSION: 2.7.2 */
#include "psyq/libcard/cardc11.h"
extern volatile int CardTxCount;
extern int CardCmdSector;
extern int CardVarRxLength;
extern int D_8011472C;
extern unsigned char *CardCmdBuffer;

int _cardCmd5CVarTx() {
    switch (CardTxCount) {
    case 2: return 0x5C;
    case 3: return CardCmdSector;
    }
    if (CardTxCount < 5) return 0;
    if (D_8011472C == 0) {
        if (CardTxCount >= CardVarRxLength + 5) return 0;
        return CardCmdBuffer[CardTxCount - 5];
    }
    return CardCmdBuffer[CardTxCount - CardVarRxLength - 6];
}
