/* GCC_VERSION: 2.7.2 */
extern volatile int CardTxCount;
extern int CardCmdSector, CardCmdValue, CardVarRxLength;
extern unsigned char *CardCmdBuffer;
int _cardCmd5BVarTx() {
    switch (CardTxCount) {
    case 2: return 0x5B;
    case 3: return CardCmdSector;
    }
    if (CardTxCount < 5) return 0;
    if (CardCmdValue != 0) return 0;
    if (CardTxCount >= CardVarRxLength + 5) return 0;
    return CardCmdBuffer[CardTxCount - 5];
}
