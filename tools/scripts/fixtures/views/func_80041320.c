#include "types.h"

#define ROW(i) ((RrRow60 *)((s32)MenuTextRows + (i) * 0x3C))

/* legacy typing of shared globals in this part of the unit */
#define AddDrawModePrim (*(unsigned *(*)())&AddDrawModePrim)

#define CurrentDrawBuf ((char *)CurrentDrawBuf)

#undef AddDrawModePrim

#undef CurrentDrawBuf

/* legacy typing of shared globals in this part of the unit */
#define MenuTextRows ((E *)MenuTextRows)

#undef MenuTextRows

/* legacy typing of shared globals in this part of the unit */
#define SetupDrawDispEnvs ((void (*)())SetupDrawDispEnvs)

#define GlyphRows ((char *)GlyphRows)

#undef SetupDrawDispEnvs

#undef GlyphRows

/* legacy typing of shared globals in this part of the unit */
#define D_800AD520 (*(unsigned char (*))&D_800AD520)

#define ModeFrame (*(unsigned (*))&ModeFrame)

#define BadgeRows ((char *)BadgeRows)

#define Dialogue ((signed char *)Dialogue)

#undef D_800AD520

#undef ModeFrame

#undef BadgeRows

#undef Dialogue

typedef struct {
    u8 unk0;
    u8 unk1;
    s8 unk2;
    s8 unk3;
    u8 unk4[24];
    u8 unk1C[24];
    u16 unk34;
    u16 unk36;
    s16 unk38;
    s16 unk3A;
} RrRow60;

typedef struct 
{
  char pad[0x1C];
  char c;
  char pad2[0x17];
  short d;
  unsigned short e;
} E;

extern RrRow60 *MenuTextRows;

extern unsigned short D_8009FDA0[];

void SetMenuTextRowCursor(a0, a1, a2)
int a0;
int a1;
int a2;
{
  E *p;
  ((E *) (((int) MenuTextRows) + (a0 * 0x3C)))->e = D_8009FDA0[a1];
  if (a2 < (-0xFF))
  {
    ((E *) (((int) MenuTextRows) + (a0 * 0x3C)))->d = -a2;
    return;
  }
  switch (a2)
  {
    case 0:
      ((E *) (((int) MenuTextRows) + (a0 * 0x3C)))->c = 0x20;
      return;

    case 1:
      ((E *) (((int) MenuTextRows) + (a0 * 0x3C)))->c = 0x3E;
      return;

    case -1:
      return;

  }

}
