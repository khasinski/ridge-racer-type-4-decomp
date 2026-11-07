#include "types.h"

#include "rr/track_sample_types.h"

extern int AddDrawModePrim();

extern int AddSpritePrim();

extern int CurrentDrawBuf;

extern unsigned char D_8009F7CC[];

extern unsigned char SelectionGridIconB[];

extern unsigned char D_8009F7D4[];

extern unsigned char SelectionGridIconCol4[];

extern unsigned char SelectionGridIcon5[];

extern struct { char pad[0x1D2]; short h1D2; char pad2[0x1F]; unsigned char b1F3; } *CarRecordPtrs[];

extern unsigned char SelectionRows[][4];

void DrawSelectionIconGrid(int arg0) {
    int v[9];
    char buf[4];
    int s0;
    int a1;
    int b;
    unsigned char *t7 = D_8009F7CC;
    unsigned char (*q)[4] = SelectionRows;
    unsigned char *p = q[arg0];
    v[0] = t7[p[1]];
    v[1] = SelectionGridIconB[p[1]];
    v[2] = CarRecordPtrs[arg0]->h1D2 > 0 ? 0x18 : 0x19;
    v[3] = D_8009F7D4[p[0]];
    v[4] = SelectionGridIconCol4[p[0]];
    v[5] = SelectionGridIcon5[p[0]];
    v[6] = CarRecordPtrs[arg0]->b1F3 + 0x14;
    b = p[3];
    v[7] = b + 0x1A;
    v[8] = b + 0x1C;
    if (p[2] == 0x27) {
        v[5] = 0x1E;
        v[4] = 0x1E;
        v[3] = 0x1E;
        v[1] = 0x1E;
        v[0] = 0x1E;
    }
    if (p[3] == 2) {
        v[8] = 0x1E;
        v[7] = 0x1E;
    }
    s0 = 0;
    a1 = *(int *)0x1F800000;
    buf[2] = 0x10;
    buf[3] = 0x20;
    do {
        int t = v[s0];
        buf[0] = (t % 16) * 16;
        buf[1] = (t / 16) << 5;
        a1 = AddSpritePrim(CurrentDrawBuf + 0x70, a1, (s0 % 3) * 0x14 + 0xF0, (s0 / 3) * 0x28 + 0x58, buf, 0x7FA4, 1);
        s0++;
    } while (s0 < 9);
    *(int *)0x1F800000 = AddDrawModePrim(CurrentDrawBuf + 0x70, a1, 0x39);
}
