#include "scratchpad.h"
#include "rr/gpu.h"
#include "rr/animated_glyph.h"
#include "game/gfx/fontdraw.h"
#include "psyq/libgpu/primattr.h"
#include "psyq/libgpu/settrans.h"
#include "rr/player_status.h"

extern unsigned char D_8009EDF8[];

char *AddTextSprites();
void DrawAnimatedGlyph(s32 x, s32 y, s32 glyph, s32 clut, s16 *animation);
void DrawAnimatedText();

/* legacy typing of shared globals in this part of the unit */
#define SetDrawMode ((s32 (*)())SetDrawMode)
#define AddPrim ((s32 (*)())AddPrim)
#define SetSprt ((s32 (*)())SetSprt)
#define CurrentDrawBuf ((char *)CurrentDrawBuf)

void DrawBigFontString(a0, a1, a2)
s32 a0, a1;
u8 *a2;
{
    RrSpritePacket *s3, *s2;
    s32 c, a, v, row, col, u, vv;
    s3 = *(RrSpritePacket **)SCRATCHPAD;
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
    *(char **)SCRATCHPAD = (char *)s3 + 0xC;
}

#undef SetDrawMode
#undef AddPrim
#undef SetSprt
#undef CurrentDrawBuf

/* legacy typing of shared globals in this part of the unit */
#define AddPrim ((void (*)())AddPrim)
#define SetSemiTrans ((void (*)())SetSemiTrans)
#define SetShadeTex ((void (*)())SetShadeTex)
#define SetSprt ((void (*)())SetSprt)

char *AddTextSprites(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7)
int arg0;
char *arg1;
int arg2;
int arg3;
unsigned char *arg4;
int arg5;
int arg6;
int arg7;
{
  int x;
  int y;
  int c;
  int h;
  int new_var;
  char *p;
  char *q;
  h = arg7 >> 1;
  if ((*arg4) != 0)
  {
    p = arg1;
    do
    {
      c = (*arg4) - 0x20;
      arg4++;
      if (c != 0)
      {
        x = FontGlyphCells[c * 2] << 3;
        y = FontGlyphCells[(c * 2) + 1] << 4;
        SetSprt(arg1);
        if (h == 0)
        {
          SetShadeTex(arg1, 1);
        }
        else
        {
          p[4] = h;
          p[5] = h;
          p[6] = h;
        }
        SetSemiTrans(arg1, arg7 & 1);
        *((short *) (p + 8)) = arg2;
        *((unsigned short *) (p + 0xA)) = arg3;
        *((short *) (p + 0x10)) = 8;
        p[0xC] = x;
        p[0xD] = y;
        *((short *) (p + 0x12)) = 0x10;
        *((unsigned short *) (p + 0xE)) = arg5;
        q = p;
        p += 0x14;
        arg1 += 0x14;
        AddPrim(arg0, q);
      }
      if (arg6 == 1)
      {
        arg2 = (arg2 + (new_var = 2)) + D_8009EDF8[((x / 8) + ((y / 16) * 32)) % 54];
      }
      else
        if (arg6 == 2)
      {
        arg2 = arg2 + D_8009EDF8[((x / 8) + ((y / 16) * 32)) % 54];
      }
      else
      {
        arg2 += 6;
      }
    }
    while (*arg4 != 0);
  }
  return arg1;
}

#undef AddPrim
#undef SetSemiTrans
#undef SetShadeTex
#undef SetSprt

void DrawAnimatedGlyph(s32 x, s32 y, s32 glyph, s32 clut, s16 *animation) {
    s16 nextTimer;
    s32 tileU,shadowBrightness;
    s32 drawV;
    s32 yStep;
    s32 row;
    s32 curve;
    s16 displacement;
    s32 pass;
    s32 baseV;
    s16 tileV;
#ifdef __mips__
    register RrSpritePacketSignedClut *sprites __asm__("$16");
#else
    RrSpritePacketSignedClut *sprites;
#endif
    u8 *cursor;
    RrSpritePacketSignedClut *linkPacket;

    cursor = RR_GLYPH_ANIMATION_SCRATCH->cursor;
    row = glyph/32;
    tileU=(glyph-row*32)*8;
    baseV = row * 0x18;
    tileV = baseV;
    if (glyph >= 0x140) {
        tileV = baseV + 0x10;
    }
    tileV=RR_GLYPH_PREF(tileV,"$22");
    displacement = 0;
    pass = 0;
    shadowBrightness = RR_GLYPH_PREF(8,"$23");
    __asm__("" : "+r"(tileU));
    sprites = (RrSpritePacketSignedClut *)cursor;
    /* The original retains a shadow branch, but runs only pass zero. */
    do {
        SetSprt(cursor);
        if (pass == 1) {
            sprites->r = shadowBrightness;
            sprites->g = shadowBrightness;
            sprites->b = shadowBrightness;
        } else {
            displacement = 0;
            if (*animation < 0x80) {
                sprites->r = (u8) *animation;
                sprites->g = (u8) *animation;
                sprites->b = (u8) *animation;
                nextTimer = (u16) *animation + 8;
                *animation = nextTimer;
                curve = *animation/8;
                curve *= curve;
                if(curve<0)curve+=7;
                curve >>= 3;
                if (*animation >= 0x31) {
                    displacement = curve - 4;
                } else {
                    displacement = -curve;
                }
            } else {
                s32 brightness=RR_GLYPH_PREF(128,"$2");
                sprites->r = brightness;
                sprites->g = brightness;
                sprites->b = brightness;
            }
        }
        __asm__("" :: "r"(tileU), "r"(shadowBrightness));
        linkPacket=sprites;
        if ((s16)displacement > 0) {
            displacement = 0;
        }
        cursor += 0x14;
        sprites->x = (s16)((u32)displacement + ((u32)x + (u32)pass));
        yStep = pass * 2;
        __asm__("" : "+r"(sprites) : "r"(yStep),"r"(y));
        sprites->u = (s8)tileU;
        sprites->v = tileV;
        sprites->y = (s16)((u32)y + (u32)yStep);
        sprites->w = 8;
        sprites->h = 0x18;
        sprites->clut = clut;
        sprites++;
        AddPrim(&CurrentDrawBuf->head70, (u32 *)linkPacket);
        pass++;
    } while (pass <= 0);
    drawV = (s16) tileV;
    if (drawV < 0) {
        drawV += 0xFF;
    }
    SetDrawMode(cursor, 0, 1, ((s32) (drawV & 0x100) >> 4) | 0x2F, 0);
    AddPrim(&CurrentDrawBuf->head70, (u32 *)cursor);
    RR_GLYPH_ANIMATION_SCRATCH->cursor = cursor + 0xC;
}

void DrawAnimatedText(arg0, arg1, p, arg3, arg4)
int arg0; int arg1; char *p; int arg3; int arg4;
{
    int s7 = 0, s4 = 0, s2 = 0, s6, s3;
    unsigned s0;
    unsigned short *s1;
    unsigned a2, v0; int v1;
    if (arg3 == 1) {
        *(short *)(p + 0x12) = (unsigned char)p[0xA] - (unsigned char)p[0xB];
        s6 = (unsigned char)p[0xA] - (unsigned char)p[0xB];
    } else {
        s6 = *(short *)(p + 0x12);
    }
    s1 = *(unsigned short **)p;
    s0 = 0;
    if (s6 != 0) {
        do {
            a2 = *s1++;
            s3 = 0;
            if (a2 & 0x1000) {
                v0 = a2 & 0xFFF;
                if (v0 == 0) {
                    s2 = s3;
                    s4 += 0x18;
                    goto L_BE8;
                }
                a2 = v0;
                s3 = 1;
                goto L_call;
            }
            v1 = a2;
            if ((a2 & 0xFE00) == 0x6000) {
                s0 -= 1;
                a2 = *s1++;
                switch (v1 & 0xFFFF) {
                default:
                    s0 += 1;
                    goto L_BEC;
                case 0x6005:
                    *(short *)(p + 0xE) = a2 & 0xFF;
                    goto L_BE8;
                case 0x6003:
                    *(char *)(p + 0x1C) = a2;
                    goto L_BE8;
                case 0x6004:
                    *(short *)(p + 0x14) = (signed char)a2;
                    if (s0 != s6 - 2) {
                        s0++;
                        goto L_BEC;
                    }
                    s0 = s6 - 1;
                    goto L_BE8;
                }
            }
            /* ASM_BARRIER: launder a2 so the second (a2 & 0xFE00) is not CSE'd with the 0x6000 test */
            __asm__("" : "+r"(a2));
            if ((int)(a2 & 0xFE00) > 0) goto L_BE8;
            if (a2 == 0) {
                s2 += 7;
                goto L_BE8;
            }
        L_call:
            DrawAnimatedGlyph(arg0 + (short)s2, arg1 + (short)s4, a2 - 1, arg4, (s16 *)(p + (s7 * 4 + 0x28)));
            s2 += 7;
            if (s3 != 0) {
                s2 = 0;
                s4 += 0x1A;
            }
            s7 += 1;
        L_BE8:
            s0 += 1;
        L_BEC:;
        } while (s0 < s6);
    }
}
