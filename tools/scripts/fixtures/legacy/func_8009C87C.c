/* GCC_VERSION: 2.7.2 */
/* RODATA 0x80012158 0x20 */
extern int CardTxCount;
extern unsigned char CardCmdSector, D_80114725, D_80114726, D_80114727;
extern int D_80114728;

int _cardCmd5BRangeTx() {
    switch (CardTxCount) {
    case 2: return 0x5B;
    case 3: return 1;
    case 5: return CardCmdSector;
    case 6: return D_80114725;
    case 7: return D_80114726;
    case 8: return D_80114727;
    case 9: return D_80114728;
    }
    return 0;
}
