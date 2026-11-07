/* CC1_FLAGS: -mno-split-addresses */
#include "psyq/libcard/cardc161.h"
extern volatile int CardTxCount;
int _cardCmd58Tx() {
 register int r asm("$2") = 0x58; int d; d = CardTxCount; if (d != 2) r = 0; return r;
}
