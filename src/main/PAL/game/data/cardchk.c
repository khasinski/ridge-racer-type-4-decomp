/* RODATA 0x8001140C 0x4c */
#include "game/data/cardini.h"
#include "game/data/cardchk.h"
extern int D_8010EF88[];
extern int D_8010EF90[];
extern unsigned char CardCheckCounts[];
extern unsigned char CardSlotState[];
extern unsigned char CardFileCounts[];

void PollCardStatus(a0, a1, a2)
int a0; int a1; int a2;
{
    int r, flag, k, s3, t, v;
    /* ASM_BARRIER: pin loop counter to s2 to reproduce original register numbering */
    register int i asm("$18");
    unsigned char *p0, *p1, *pp, *b0;

    k = a0 & 0xFF;
    r = CardUpdate(k, a1 & 0xFF);
    D_8010EF88[k] = r;
    if (r != 0) {
        D_8010EF90[k] = r;
        switch (r) {
        case -2: CardCheckCounts[0]++; break;
        case -1: CardCheckCounts[1]++; break;
        case 1: CardCheckCounts[2]++; break;
        case 2: CardCheckCounts[3] += 3; break;
        case -3: CardCheckCounts[4]++; break;
        case 3: CardCheckCounts[5]++; break;
        case 4: CardCheckCounts[6] += 3; break;
        case -4: CardCheckCounts[7]++; break;
        default: CardCheckCounts[8]++; break;
        }
    } else {
        D_8010EF90[k] = 0;
    }
    i = 0;
    flag = 0;
    b0 = CardSlotState;
    s3 = a0 & 0xFF;
    p0 = b0 + s3;
    p1 = D_8010B2DC + s3;
loop:
    if (CardCheckCounts[i] >= 3) {
        switch (i) {
        case 0:
            *p0 = 0x14; *p1 = 0; flag = 1; break;
        case 1:
            *p0 = 0x1E; *p1 = 0; flag = 1; break;
        case 2:
            if (*p0 >= 0x10) {
                CardFileCounts[s3] = CountCardFiles(s3, a1 & 0xFF);
                *p0 = 0xF - CardFreeBlocks(CardFileCounts[s3]);
                *p1 = 0;
            }
            flag = 1; break;
        case 3:
            if (*p0 >= 0x10) { *p0 = 0x32; *p1 = 0; }
            flag = 1; break;
        case 4:
            *p0 = 0x14; *p1 = 1; flag = 1; break;
        case 5:
            if (*p0 >= 0x10) {
                CardFileCounts[s3] = CountCardFiles(s3, a1 & 0xFF);
                *p0 = 0xF - CardFreeBlocks(CardFileCounts[s3]);
                *p1 = 1;
            }
            flag = 1; break;
        case 6:
            if (*p0 >= 0x10) { *p0 = 0x32; *p1 = 1; }
            flag = 1; break;
        case 7:
            if (*p0 >= 0x10) { *p0 = 0x28; *p1 = 0; }
            flag = 1; break;
        case 8:
            if (*p0 >= 0x10) { *p0 = 0x3C; *p1 = 0; }
            flag = 1; break;
        }
    }
next:
    i++;
    if (i < 9) goto loop;
    if (flag == 1 || (short)a2 == 1) {
        i = 8; pp = CardCheckCounts + i; do { *pp-- = 0; } while (--i >= 0);
    }
}
