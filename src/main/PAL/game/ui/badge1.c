#include "types.h"
#include "rr/badge_rows.h"
#include "rr/draw_environment.h"
#include "m2c_macros.h"
#include "psyq/libgpu/primatt5.h"
#include "rr/animated_glyph.h"
#include "rr/hud_number.h"

struct P12 { int a, b, c; };

typedef struct { int a, b, c; } T;

extern int SetDrawArea();
extern char FrameEnvs[];

void *AddDrawAreaPrim(u32 *a0, void *cursor, int a2, int a3, int a4, int a5);
T *AddDrawModePrim(u32 *a0, T *a1, unsigned short a2);
void *AddTilePrim(u32 *arg0, void *arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6, s32 arg7, s32 arg8);
void *AddTileGrid2x2(u32 *a0, void *a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8);

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

/* legacy typing of shared globals in this part of the unit */
#define AddPrim ((void (*)(u32 *,u32 *))AddPrim)

T *AddDrawModePrim(u32 *a0, T *a1, unsigned short a2) {
    SetDrawMode(a1, 0, 1, a2, 0);
    AddPrim(a0, (u32 *)a1++);
    return a1;
}

#undef AddPrim

void *AddTilePrim(u32 *arg0, void *arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6, s32 arg7, s32 arg8) {
    SetTile(arg1);
    M2C_FIELD(arg1, s16 *, 8) = arg2;
    M2C_FIELD(arg1, s16 *, 0xA) = arg3;
    M2C_FIELD(arg1, s16 *, 0xC) = (s16) arg4;
    M2C_FIELD(arg1, s16 *, 0xE) = (s16) arg5;
    M2C_FIELD(arg1, s8 *, 4) = (s8) arg6;
    M2C_FIELD(arg1, s8 *, 5) = (s8) arg7;
    M2C_FIELD(arg1, s8 *, 6) = (s8) arg8;
    AddPrim(arg0, arg1);
    return arg1 + 0x10;
}

void *AddTileGrid2x2(u32 *a0, void *a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8)
{
    int i = 0;
    int w = a4 / 2;
    int h = a5 / 2;
    do {
        int x = a2 + w * (i & 1);
        int y = a3;
        if (i >= 2)
            y += h;
        a1 = AddTilePrim(a0, a1, x, y, w, h, a6, a7, a8);
        i++;
    } while (i < 4);
    return a1;
}
