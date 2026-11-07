/* GCC_VERSION: 2.7.2 */
extern int CardRxCount, CardBytesRemaining;
int _cardRxStatusByte(a0) int a0; { if (CardRxCount == 3) CardBytesRemaining = a0 & 0xFF; return 0; }
