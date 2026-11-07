/* GCC_VERSION: 2.7.2 */
extern int CardTxCount, CardCmdSector, CardVarRxLength;
int _cardCmd5ETx() {
    int v = CardTxCount;
    int r;
    if (v == 4) goto c4;
    if (v < 5) {
        if (v == 2) goto c2;
        r = 0;
        goto end;
    }
    if (v == 5) goto c5;
    r = 0;
    goto end;
c2:
    r = 0x5E;
    goto end;
c4:
    r = CardCmdSector;
    goto end;
c5:
    r = CardVarRxLength;
end:
    return r;
}
