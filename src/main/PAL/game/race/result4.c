/* RODATA 0x800106d0 0x1c */
#include "types.h"
#include "scratchpad.h"
#include "rr/ui.h"
#include "m2c_macros.h"
#include "game/menu/menuwork.h"
#include "game/ui/barsel0.h"
#include "rr/audio_loop.h"
#include "rr/menu_background.h"
#include "game/ui/barsel0.h"
#include "game/data/globals.h"
#include "game/gfx/drawbuf.h"

typedef struct { short a, b; } X;

typedef struct { char p[4]; char a; char b; char c; char d; } Y;

extern int AddDrawModePrim();
extern int AddSpritePrim();
extern unsigned char ResultGlyphUvRects[];
extern unsigned char ResultGlyphIndexTable[];
extern unsigned short CourseMenuSelection;
extern int InitBadgeRow();
extern int InitRect16Row();
extern int InitGlyphRow();
extern int LoadCursor;
extern int ModeFrame;
extern int MenuArenaBase;
extern signed char LoadState;
extern int GlyphRows;
extern s32 VSyncThreshold;
extern s16 ModeId;
extern s16 ModeGroup;
extern s16 ModeSubState;
extern int DebugPrintStub();
extern int SetMenuFrameOptions();
extern void UpdateCourseListChoice(void);
extern int InitMenuArenas();
extern int LoadCarRangeStep();
extern int DrawUiRow();
extern char NewCar0Msg[];
extern char StrNewCarLog[];
extern int MenuStepDirection;
extern int MenuConfirmPressed;
extern struct { char pad[0x10]; short h10; } *BadgeRows;
extern struct { char pad[4]; signed char b4; char b5; signed char b6; } *Dialogue;

void DrawResultsTitleBanner();
void InitNewCarScreen(int a0);
void NewCarModeEnter(void);
void NewCarModeUpdate(void);

void DrawResultsTitleBanner(a0)
int a0;
{
  int s4;
  int a1;
  int sz2;
  int new_var;
  int s3;
  int s1;
  int s2;
  int s0;
  int t0;
  int d;
  /* ASM_BARRIER: pin t to $v1 like the original */
  register int t asm("$3");
  int u;
  unsigned char *p;
  unsigned char *q;
  unsigned char *base;
  s4 = 0x80;
  if (a0 < 0x41)
  {
    s4 = a0 * 2;
  }
  a1 = *((int *) SCRATCHPAD);
  s3 = 0x50;
  s1 = 0;
  do
  {
    if (s1 < 2)
    {
      d = RaceType;
      if ((d != 1) && (s1 == 1))
      {
        goto next;
      }
      switch (d)
      {
        case 1:
          s2 = ResultGlyphIndexTable[(D_800AD4EF * 2) + s1];
          break;

        case 0:
          s2 = 3;
          break;

        case 2:

        case 3:
          s2 = 0;
          break;

        case 4:
          s2 = 1;
          break;

        case 6:
          s2 = 2;
          break;

      }

    }
    if (s1 == 2)
    {
      s2 = 0xC;
    }
    else
      if (s1 == 3)
    {
      t = s2 + 1;
      s2 = t + (CourseMenuSelection & 7);
    }
    p = ResultGlyphUvRects + (s2 * 4);
    a1 = AddSpritePrim(CurrentDrawBuf + 0x70, a1, s3, 0x48, p, 0x263A, (s4 + 1) * 2);
    u = s3 + 2;
    s3 = u + p[2];
    next:
    s1++;

  }
  while (s1 < 4);
  s2 = 0x16;
  if (RaceType != 1)
  {
    s0 = 1;
    s3 = 0xD6;
  }
  else
  {
    s2 = 0x15;
    s0 = 2;
    s3 = 0xE6;
  }
  t0 = 0x263A;
  s1 = 0;
  if (s0 != 0)
  {
    base = ResultGlyphUvRects;
    do
    {
      new_var = s3 - 2;
      a1 = AddSpritePrim(CurrentDrawBuf + 0x70, a1, s3, 0x24, (unsigned char *)((s2 * 4) + (int)base), t0, (s4 + 1) * 2);
      s2 = 0x17;
      s1++;
      q = base + 0x5C;
      t = new_var;
      s3 = t - q[2];
      t0 = 0x263B;
    }
    while (s1 < s0);
  }
  *((int *) SCRATCHPAD) = AddDrawModePrim(CurrentDrawBuf + 0x70, a1, 0x2E);
}

/* legacy typing of shared globals in this part of the unit */
#define BadgeRows ((X *)BadgeRows)
#define Dialogue ((Y *)Dialogue)

void InitNewCarScreen(int a0) {
    int i = 0;
    int a;
    int t;
    int p;
    do {
        a = a0 + i;
        t = a * 0x1C;
        *(int *)(t + GlyphRows + 0x18) = *(int *)(i * 0x1C + GlyphRows + 0x18) + 0x30000;
        InitGlyphRow(a, -2, 0, 0);
        i += 1;
        p = t + GlyphRows;
        InitRect16Row(*(unsigned char *)(p + 7), *(short *)(p + 8), *(short *)(p + 0xA), *(short *)(p + 0xC), *(short *)(p + 0xE), 0, 0);
    } while (i < 4);
    InitBadgeRow(0, 0xC, 0x130, 0x128, 0x58, 0, 0);
    InitUiRow(0, 0x28, 0x48, 0xC0, 0x20, 0, 0, 4, 3, 4);
    BadgeRows->b = 1;
    Dialogue->a = 1;
    Dialogue->d = 0;
    ModeFrame = 0;
    LoadState = 0;
    LoadCursor = MenuArenaBase;
}

#undef BadgeRows
#undef Dialogue

void NewCarModeEnter(void) {
    UpdateScreenFade(0x10006);
    InitNewCarScreen(0);
    ModeFrame = 0;
    VSyncThreshold = 0x80;
    ModeGroup = 1;
    ModeId = 0xA;
    ModeSubState = 0;
}

/* legacy typing of shared globals in this part of the unit */
#define ModeFrame (*(unsigned (*))&ModeFrame)

void NewCarModeUpdate(void) {
    int a;
    int t;
    int r;
    int s1;
    ModeFrame += 1;
    if (Dialogue->b4 >= 0x22) {
        DrawUiRow(0);
        a = 0x10006;
    } else {
        a = 0x10007;
    }
    UpdateScreenFade(a);
    if (CourseListMenuStep(MenuStepDirection) != 0) {
        if (BadgeRows->h10 == 3) {
            UpdateCourseListChoice();
            return;
        }
    }
    if (Dialogue->b4 == 1) {
        if (MenuConfirmPressed != 0 || ModeFrame >= 0x1F5) {
            t = Dialogue->b6;
            if (t == 4) {
                Dialogue->b4 = 0x22;
                BadgeRows->h10 = t;
                return;
            }
        }
    }
    if (D_800AD4F2 == 2) {
        r = LoadCarRangeStep(0, 1);
        if (r == 1) {
            if (BadgeRows->h10 == 0xB) {
                SetMenuFrameOptions(0, 1, 0);
                DebugPrintStub(NewCar0Msg);
                s1 = LoadCursor;
                InitMenuArenas();
                CarPreviewSetup();
                ModeGroup = r;
                ModeId = 0x22;
                ModeSubState = 0;
                LoadCursor = s1;
                DebugPrintStub(StrNewCarLog);
            }
        }
    }
}

#undef ModeFrame
