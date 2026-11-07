/* GCC_VERSION: 2.7.2 */
#include "psyq/libcard/cardc09.h"
extern volatile int CardTxCount;
extern int CardCmdSector;
extern int D_8011472C;
extern int CardVarRxLength;
extern unsigned char *CardCmdBuffer;
int _cardCmd5BVarTx() {
    switch (CardTxCount) {
    case 2: return 0x5B;
    case 3: return CardCmdSector;
    }
    if (CardTxCount < 5) return 0;
    if (D_8011472C != 0) return 0;
    if (CardTxCount >= CardVarRxLength + 5) return 0;
    return CardCmdBuffer[CardTxCount - 5];
}
