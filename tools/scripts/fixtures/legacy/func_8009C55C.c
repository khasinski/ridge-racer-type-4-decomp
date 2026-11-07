/* GCC_VERSION: 2.7.2 */
extern volatile int CardTxCount;
extern unsigned char CardCmdSector, CardCmdSectorByte1;
extern unsigned char *CardCmdBuffer;

int _cardCmd59ParamTx() {
    switch (CardTxCount) {
    case 2: return 0x59;
    case 4: return CardCmdSectorByte1;
    case 5: return CardCmdSector;
    }
    if (CardTxCount < 6) return 0;
    if (CardTxCount >= 10) return 0;
    return CardCmdBuffer[CardTxCount - 6];
}
