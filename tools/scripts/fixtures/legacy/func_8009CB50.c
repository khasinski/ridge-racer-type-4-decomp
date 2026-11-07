/* GCC_VERSION: 2.7.2 */
extern int CardRxCount, D_80114728, CardBytesRemaining;
int _cardCmd5CRangeRx() { if (CardRxCount == 9) CardBytesRemaining = D_80114728 + 1; return 0; }
