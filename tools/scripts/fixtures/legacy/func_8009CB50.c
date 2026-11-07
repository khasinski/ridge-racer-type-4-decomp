/* GCC_VERSION: 2.7.2 */
extern int CardRxCount, CardVarRxLength, CardBytesRemaining;
int _cardCmd5CRangeRx() { if (CardRxCount == 9) CardBytesRemaining = CardVarRxLength + 1; return 0; }
