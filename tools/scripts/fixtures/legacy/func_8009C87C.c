/* GCC_VERSION: 2.7.2 */
/* RODATA 0x80012158 0x20 */
extern int CardTxCount;
extern unsigned char CardCmdSector, CardCmdSectorByte1, CardCmdSectorByte2, CardCmdSectorByte3;
extern int CardVarRxLength;

int _cardCmd5BRangeTx() {
    switch (CardTxCount) {
    case 2: return 0x5B;
    case 3: return 1;
    case 5: return CardCmdSector;
    case 6: return CardCmdSectorByte1;
    case 7: return CardCmdSectorByte2;
    case 8: return CardCmdSectorByte3;
    case 9: return CardVarRxLength;
    }
    return 0;
}
