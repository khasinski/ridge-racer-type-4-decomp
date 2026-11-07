/* GCC_VERSION: 970318 */
/* RODATA 0x800121a0 0x3c */
extern int CardRxCount;
extern int CardBytesRemaining;
extern char *CardCmdBuffer;
int _cardCmd5ARx(a0) int a0; {
    int v;
    int r;
    char *c;
    switch (CardRxCount) {
    case 3:
        CardBytesRemaining = a0 & 0xFF;
        break;
    case 4:
        CardCmdBuffer[1] = a0;
        break;
    case 5:
        CardCmdBuffer[0] = a0;
        break;
    case 6:
        CardCmdBuffer[0x10] = a0;
        break;
    case 17:
        CardCmdBuffer[0xC] = a0 - 1;
        break;
    default:
        v = CardRxCount;
        if ((unsigned)(v - 9) < 4) {
            c = CardCmdBuffer + 0x10;
            c -= v;
            *c = a0;
            break;
        }
        if (v < 7) break;
        CardCmdBuffer[v - 5] = a0;
        break;
    }
    r = 0;
out:
    return r;
}
