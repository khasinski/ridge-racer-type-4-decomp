/* GCC_VERSION: 2.7.2 */
/* RODATA 0x80012178 0x24 */
extern volatile int CardTxCount;
extern int CardCmdBuffer, CardVarRxLength;
extern unsigned char CardCmdSector, D_80114725, D_80114726, D_80114727;
int _cardCmd5CRangeTx() {
    switch (CardTxCount) {
    case 2: return 0x5C;
    case 3: return 1;
    case 5: return CardCmdSector;
    case 6: return D_80114725;
    case 7: return D_80114726;
    case 8: return D_80114727;
    case 9: return CardVarRxLength;
    case 4:
    case 10: break;
    default: return *(unsigned char *)(CardTxCount + CardCmdBuffer - 0xB);
    }
    return 0;
}
