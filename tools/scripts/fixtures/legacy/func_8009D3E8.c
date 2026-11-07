/* GCC_VERSION: 2.7.2 */
extern int CardTxCount, CardCmdSector;
int _cardCmd5FTx() {
    switch (CardTxCount) {
    case 2: return 0x5F;
    case 4: return CardCmdSector;
    }
    return 0;
}
