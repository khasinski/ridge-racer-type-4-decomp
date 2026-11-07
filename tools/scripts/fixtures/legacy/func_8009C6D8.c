/* GCC_VERSION: 2.7.2 */
extern int CardTxCount;
int _cardCmd5BTx() {
    register int r asm("$2");
    register int k asm("$2");
    int d = CardTxCount;
    k = 2;
    if (d == k) r = 0x5B;
    else r = 0;
    return r;
}
