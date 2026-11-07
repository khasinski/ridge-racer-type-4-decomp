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

extern unsigned char D_8009EDF8[];

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
