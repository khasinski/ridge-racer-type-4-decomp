#include "rr/badge_rows.h"

/* legacy typing of shared globals in this part of the unit */
#define AddPrim ((void (*)(u32 *,u32 *))AddPrim)

#undef AddPrim

struct P12 { int a, b, c; };

extern int SetDrawArea();

extern int AddPrim();

extern char FrameEnvs[];

extern int DrawBufIndex;

void *AddDrawAreaPrim(u32 *a0, void *cursor, int a2, int a3, int a4, int a5) {
    struct P12 *a1=cursor;
    short sp[4];
    char *p = FrameEnvs;
    int t;
    sp[0] = a2;
    if (*(short *)(p + 0x5E) == *(short *)(p + 0x227D6)) {
        sp[1] = a3;
    } else {
        t = DrawBufIndex;
        sp[1] = a3 + (t << 8);
    }
    sp[2] = a4;
    sp[3] = a5;
    SetDrawArea(a1, sp);
    AddPrim(a0, a1++);
    return a1;
}
