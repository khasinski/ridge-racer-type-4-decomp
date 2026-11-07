#include "rr/gpu.h"

#include "rr/animated_glyph.h"

/* legacy typing of shared globals in this part of the unit */
#define SetDrawMode ((s32 (*)())SetDrawMode)

#define AddPrim ((s32 (*)())AddPrim)

#define SetSprt ((s32 (*)())SetSprt)

#define CurrentDrawBuf ((char *)CurrentDrawBuf)

#undef SetDrawMode

#undef AddPrim

#undef SetSprt

#undef CurrentDrawBuf

/* legacy typing of shared globals in this part of the unit */
#define AddPrim ((void (*)())AddPrim)

#define SetSemiTrans ((void (*)())SetSemiTrans)

#define SetShadeTex ((void (*)())SetShadeTex)

#define SetSprt ((void (*)())SetSprt)

#undef AddPrim

#undef SetSemiTrans

#undef SetShadeTex

#undef SetSprt

extern s32 SetSemiTrans();

extern s32 SetShadeTex();

extern u8 FontGlyphCells[];

void DrawBigFontString(a0, a1, a2)
s32 a0, a1;
u8 *a2;
{
    RrSpritePacket *s3, *s2;
    s32 c, a, v, row, col, u, vv;
    s3 = *(RrSpritePacket **)0x1F800000;
    s2 = s3;
    for (; *a2 != 0; a0 += 10) {
            a = *a2 - 0x20;
            a2++;
            if (a != 0) {
                a = FontGlyphCells[a * 2] + (FontGlyphCells[a * 2 + 1] << 5);
                row = a / 21;
                col = a % 21;
                u = col * 12;
                vv = row * 24 + 0x24;
                SetSprt(s3);
                s2->w = 12;
                s2->h = 24;
                SetShadeTex(s3, 1);
                SetSemiTrans(s3, 1);
                s2->x = a0;
                s2->y = a1;
                s2->u = u;
                s2->v = vv;
                s2->clut = 0x5B20;
                AddPrim(CurrentDrawBuf + 0x70, s2++);
                s3++;
            }
    }
    SetDrawMode(s3, 0, 1, 0x58, 0);
    AddPrim(CurrentDrawBuf + 0x70, s3);
    *(char **)0x1F800000 = (char *)s3 + 0xC;
}
