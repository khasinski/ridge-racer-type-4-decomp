#include "types.h"
#include "scratchpad.h"
#include "m2c_macros.h"
#include "game/gfx/gfxmtx.h"
#include "game/menu/menustat.h"
#include "rr/menu_background.h"
#include "rr/frame_environment.h"
#include "game/data/ldscn1.h"
#include "game/gfx/gfxotag.h"
#include "game/menu/menu8.h"
#include "game/menu/menucalc.h"
#include "game/data/globals.h"
#include "game/gfx/drawbuf.h"
#include "game/data/screenframe.h"

extern int AddSpriteWithDrawMode();
extern unsigned char BadgeUvTable[][4];
extern signed char BadgePatternTable[][8];
extern unsigned char D_8009F1E0[];
extern unsigned char D_8009F1E4[];
extern struct 
{
  char pad[0x17];
  unsigned char v;
} SelectionState;
extern int AddTilePrim();
extern int rsin();
extern unsigned char D_8009F1EC[];
extern int ModeFrame;
extern int SetMenuFrameOptions();
extern int InitBadgeRow();
extern int UpdateBadgeRowAnim();
extern int InitRect16Row();
extern int DrawUiRow();
extern int InitImageRow();
extern int UpdateImageRow();
extern short ModeId;
extern short ModeGroup;
extern int D_800F4CA8;
extern unsigned char *UiRows;
extern short D_800F4D8A;
extern s32 VSyncThreshold;
extern s16 SceneId;
extern int AddScreenFadeTiles();
extern int ResetScreenCounters();
extern int PlaySfx();
extern int SeqFadeVolume();
extern int SeqMuteVolume();
extern unsigned short MenuFadeTimer;
extern int LoadCursor;
extern struct { char pad[1]; signed char b1; } *ImageRows;
extern struct { char pad[8]; unsigned short h8; char pad2[0x1E]; int w28; } PlayerControllers;
extern struct { char pad[0x10]; short h10; char pad2[10]; } *BadgeRows;
extern int LoadCursorBase;
extern struct { char pad[6]; signed char b6; char pad2[0xF]; short h16; } *Dialogue;

int func_80030D9C(int x, int a1);
int DrawStepProgressBar(int a0, int a1);
void func_80031064(void);
void func_800311FC(void);
void func_80031244(void);
void AddScreenBackdropTile(void);

int func_80030D9C(int x, int a1)
{
  int s3 = 0x78;
  unsigned char (*new_var)[4];
  int i = 0;
  int s0;
  int s2;
  int a;
  do
  {
    s2 = a1 - (i * 0x32);
    if (s2 >= 0x81)
    {
      s2 = 0x80;
    }
    else
      if (s2 < 0)
    {
      s2 = 0;
    }
    s0 = ((signed char) SelectionState.v) >> 1;
    if (s0 >= 3)
    {
      s0 = 2;
    }
    if (i == 2)
    {
      s0 += 0xB;
      a = s0;
    }
    else
    {
      a = s0;
      if (i == 1)
      {
        s0 = ((signed char) SelectionState.v) + 3;
        a = s0;
      }
    }
    s3 += D_8009F1E0[i];
    x = AddBadgeSpriteStrip(a, CurrentDrawBuf + 0x70, x, 0x20, s3, s2);
    if (i == 1)
    {
      unsigned char *t;
      int b = BadgePatternTable[s0][4];
      s0 = D_8009F1E4[(signed char) SelectionState.v] + 0x20;
      new_var = BadgeUvTable;
      t = new_var[s0];
      x = AddSpriteWithDrawMode(CurrentDrawBuf + 0x70, x, b + 0x34, s3, t[2], t[3], t[0], t[1], 0x4A, 0x2E28, i, s2);
    }
    i++;
  }
  while (i < 3);
  return x;
}

int DrawStepProgressBar(int a0, int a1) {
    int s0 = 0x28;
    int sel = D_800AD4EF;
    int i = 0;
    int v, y;
    do {
        s0 += D_8009F1EC[i];
        if (sel == i) {
            v = rsin((ModeFrame << 6) & 0xFC0) / 64 + 0xBF;
        } else if (i < sel) {
            v = 0x80;
        } else {
            s0 += 8;
            goto next;
        }
        y = s0;
        s0 += 8;
        a1 = AddTilePrim(CurrentDrawBuf + 0x70, a1, y, 0x190, 8, 0x1C, v, v, v);
next:
        i++;
    } while (i < 8);
    return a1;
}

/* legacy typing of shared globals in this part of the unit */
#define ImageRows ((char *)ImageRows)
#define ModeFrame (*(unsigned int (*))&ModeFrame)
#define BadgeRows ((short *)BadgeRows)

void func_80031064(void)
{
    int t;
    if (ModeFrame <= 0x7FFF) {
        ModeFrame += 1;
    }
    UpdateScreenFade(0x1000B);
    if (BadgeRows[8] >= 4) {
        UpdateBadgeRowAnim(0);
    }
    if (UiRows[0x14] >= 3) {
        DrawUiRow(0);
    }
    if (((signed char *)ImageRows)[1] >= 3) {
        UpdateImageRow(0);
    }
    if (ModeFrame >= 3) {
        D_800F4CA8 = 1;
        t = LoadDialogueStep();
        if (t == 1) {
            if (func_8003D108(3) == 3) {
                ModeFrame = 0;
                InitBadgeRow(0, 0xC, 0x130, 0x128, 0x6C, 0, 0);
                InitRect16Row(0, 0x10, 0x48, 0x120, 0x120, 0, 0);
                InitImageRow(0, -1, 0, 0);
                ModeGroup = t;
                ModeId = 0xC;
                D_800F4D8A = 0;
            }
        }
    } else {
        SetMenuFrameOptions(0, 1, 0);
    }
}

#undef ImageRows
#undef ModeFrame
#undef BadgeRows

void func_800311FC(void) {
    SetupDrawDispEnvs(0, 0, 0, 2);
    ModeFrame = 0;
    VSyncThreshold = 0x80;
    SceneId = 0x10C;
}

/* legacy typing of shared globals in this part of the unit */
#define D_800F4D8A (*(unsigned short (*))&D_800F4D8A)

void func_80031244(void) {
    int s0;
    void *tab;
    int a1;
    ModeFrame += 1;
    ScreenFrame += 1;
    if (UpdateImageRow(0) > 0) {
        UpdateDialoguePanel();
    }
    UpdateDialogueBox();
    if (Dialogue->b6 == 4 || (Pad0Buttons & 0x800)) {
        s0 = BadgeRows[Dialogue->h16].h10;
        if (s0 == 3) {
            if (PlayerControllers.w28 != 0 || (PlayerControllers.h8 & 0x800)) {
                PlaySfx(1);
                SetMenuFrameOptions(0, 1, 0);
                tab = BadgeRows;
                ((typeof(BadgeRows))tab)[Dialogue->h16].h10 = 4;
                ImageRows->b1 = s0;
                MenuFadeTimer = 0;
                if (PendingModeId == 0x10F || PendingModeId == 0x12A || PendingModeId == 0x205) {
                    MenuFadeTimer = 0x40;
                    ResetScreenCounters(tab);
                    LoadCursor = LoadCursorBase;
                    LoadTimListStep(0);
                    SeqFadeVolume(0, (short)(MenuFadeTimer - 4));
                    D_800F4D8A = 1;
                } else if (PendingModeId == 0x111) {
                    MenuFadeTimer = 0x40;
                    SeqFadeVolume(0, 0x3C);
                    D_800F4D8A = 1;
                }
            }
        }
    }
    if (D_800F4D8A == 1) {
        if ((short)MenuFadeTimer > 0) {
            MenuFadeTimer -= 1;
        }
        AddScreenFadeTiles(CurrentDrawBuf + 0x70, (0x40 - (short)MenuFadeTimer) * 4, 0x45);
    }
    if (BadgeRows[Dialogue->h16].h10 == 0xB) {
        if (ImageRows->b1 == 5) {
            if (PendingModeId == 0x10F || PendingModeId == 0x12A) {
                if (LoadTimListStep(1) != 1) goto done;
                SeqMuteVolume();
            if ((short)MenuFadeTimer <= 0) {
                unsigned c = PendingModeId;
                D_800F4D8A = 0;
                ModeGroup = c >> 8;
                ModeId = c & 0xFF;
            }
            } else if (PendingModeId == 0x205) {
                SeqMuteVolume();
            if ((short)MenuFadeTimer <= 0) {
                unsigned c = PendingModeId;
                D_800F4D8A = 0;
                ModeGroup = c >> 8;
                ModeId = c & 0xFF;
            }
            } else {
            if ((short)MenuFadeTimer <= 0) {
                unsigned c = PendingModeId;
                D_800F4D8A = 0;
                ModeGroup = c >> 8;
                ModeId = c & 0xFF;
            }
            }
        }
    }
done:
    UpdateScreenFade(0x1000A);
}

#undef D_800F4D8A

void AddScreenBackdropTile(void) {
    int *g = (int *)SCRATCHPAD;
    int b = *g;
    *g = AddTilePrim(CurrentDrawBuf + 0xB6C, b, 0, 0, 0x140, 0x200, 0x20, 0x20, 0x20);
}
