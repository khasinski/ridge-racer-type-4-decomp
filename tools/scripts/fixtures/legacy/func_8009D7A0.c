/* CC1_FLAGS: -mno-split-addresses */
extern volatile int CardTxCount;
int _cardCmd5ATx() {
 register int r asm("$2") = 0x5A; int d; d = CardTxCount; if (d != 2) r = 0; return r;
}
