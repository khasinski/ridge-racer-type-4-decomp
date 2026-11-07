#include "scratchpad.h"
#include "game/menu/menucalc.h"
#include "game/menu/menustat.h"
#include "game/menu/menuicon.h"
#include "game/gfx/polygt4.h"
#include "game/data/globals.h"
#include "game/gfx/drawbuf.h"
#include "game/data/menuvars.h"
extern int InitBadgeRow();
extern int UpdateBadgeRowAnim();
extern int AddDrawAreaPrim();
extern int AddTilePrim();
extern int AddSpriteWithDrawMode();
extern int func_80030BEC();
extern int BuildCameraMatrices();
extern int ScaleMatrixRow1();
extern int SetGeomOffset();
extern int SetGeomScreen();
extern short D_8009F1A0[];
extern short D_8009F1B0[];
extern short D_8009F1C0[];
extern short D_8009F1D0[];
typedef struct { short h0, h2, h4, h6, h8, ha, hc, he, h10, h12, h14, h16, h18, h1a, h1c, h1e, h20, h22, h24, h26, h28, h2a, h2c, h2e, h30, h32, h34, h36; } E1C;
typedef struct { short h0, h2, h4, h6, h8, ha, hc, he, h10, h12, h14, h16, h18, h1a; } E28;
extern char *BadgeRows;
#define G ((E1C *)BadgeRows)

void func_800307B4(a0) int a0;
{
    int s0, s1, t0;
    char *p;
    int one;
    s1 = *(int *)SCRATCHPAD;
    if (a0 < 2 && D_800F4D8A == 0) {
        s0 = 0;
        do {
            t0 = (s0 == 1 || s0 == 7) ? 8 : 0;
            InitBadgeRow(s0, D_8009F1A0[s0], D_8009F1B0[s0], D_8009F1C0[s0], D_8009F1D0[s0], (short)t0, -1);
            s0++;
        } while (s0 < 8);
        one = 1;
        s0 = 7;
        p = BadgeRows + 0xC4;
        do {
            *(short *)(p + 2) = one;
            s0--;
            p -= 0x1C;
        } while (s0 >= 0);
        goto end;
    }
    s0 = 0;
    do {
        UpdateBadgeRowAnim(s0);
        s0++;
    } while (s0 < 8);
    s1 = AddDrawAreaPrim(CurrentDrawBuf + 0x70, s1, 0, 0, 0x140, 0x200);
    s1 = AddTilePrim(CurrentDrawBuf + 0x70, s1, 0x24, 0x60, 0xF8, 2, 8, 8, 8);
    s1 = AddSpriteWithDrawMode(CurrentDrawBuf + 0x70, s1, G->h4 + 0x18, G->h6, 0x1C, 0x20, 0x7C, 0x3C, 0x4A, 0x2E29, 0, 0x80);
    s1 = AddSpriteWithDrawMode(CurrentDrawBuf + 0x70, s1, G->h4 + 0x38, G->h6, 0xA8, 0x20, 0, 0, 0x4A, 0x2E28, 1, 0x80);
    s1 = AddDrawAreaPrim(CurrentDrawBuf + 0x70, s1, G->h14, G->h16, G->h18, G->h1a);
    s1 = func_80030BEC(s1, 0x12, 2);
    s1 = AddBadgeSpriteStrip(0xE, CurrentDrawBuf + 0x70, s1, G->h20, G->h22, 0x80);
    s1 = AddBadgeSpriteStrip(0xF, CurrentDrawBuf + 0x70, s1, G->h20 + 0x90, G->h22, 0x80);
    s1 = AddBadgeSpriteStrip(0x10, CurrentDrawBuf + 0x70, s1, G->h20, G->h22, 0x80);
    s1 = AddBadgeSpriteStrip(0x11, CurrentDrawBuf + 0x70, s1, G->h20 + 0x90, G->h22, 0x80);
    s1 = DrawStepProgressBar(CurrentDrawBuf + 0x70, s1);
    s1 = AddDrawAreaPrim(CurrentDrawBuf + 0x70, s1, G->h30, G->h32, G->h34, G->h36);
    if (D_800AD4EF >= 4) {
        *(int *)SCRATCHPAD = s1;
        BuildCameraMatrices();
        ScaleMatrixRow1(2, SCRATCH_ROT_MATRIX);
        SetGeomOffset(0xA0, 0x100);
        SetGeomScreen(0x100);
        DrawMenuGridMesh(0, CurrentDrawBuf + 0x74, 1);
        s1 = *(int *)SCRATCHPAD;
        s1 = AddDrawAreaPrim(CurrentDrawBuf + 0x74, s1, *(short *)(BadgeRows + 0xD8), *(short *)(BadgeRows + 0xDA), *(short *)(BadgeRows + 0xDC), *(short *)(BadgeRows + 0xDE));
    }
    if (a0 >= 0x3D) {
        s1 = func_80030D9C(s1, G->h10 == 3 ? (a0 - 0x3C) * 2 : 0);
    }
end:
    *(int *)SCRATCHPAD = s1;
}
