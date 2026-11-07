/* GCC_VERSION: 2.7.2 */
extern int D_801102FC;
extern int StRingSize;
extern char *StRingHeaders;
int StFreeRing(int a) {
    int i, j, k;
    char *t;
    unsigned short m;
    i = ((a - ((int)StRingHeaders + (StRingSize << 5))) >> 2) / 504;
    t = StRingHeaders + (i << 5);
    m = *(unsigned short *)(t + 6);
    if (*(short *)t == 4) {
        j = 0;
        if ((short)m > 0) {
            do { k = j + i; j++; *(short *)(StRingHeaders + (k << 5)) = 0; } while (j < (short)m);
        }
        D_801102FC = j + i;
        return 0;
    }
    return 1;
}
