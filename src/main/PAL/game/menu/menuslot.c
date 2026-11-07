#include "scratchpad.h"
#include "game/menu/menu8.h"
#include "game/menu/menuslot.h"
#include "rr/menu_background.h"
#include "rr/menu_text_state.h"
#include "rr/bios_devices.h"
#include "game/data/ldscn1.h"
#include "game/gfx/drawbuf.h"
#include "game/data/screenframe.h"
#include "game/data/menuvars.h"
extern int AddScreenFadeTiles();
extern int StepMenuValue();
extern int ResetScreenCounters();
extern int func_8003B934();
extern int UpdateBadgeRowAnim();
extern int DrawUiRow();
extern int PlaySfx();
extern short MenuFadeTimer;
extern short MenuCursor;
extern short ModeId;
extern unsigned short D_800F4D8A;
extern int LoadCursor;
extern int ModeFrame;
extern int MenuConfirmPressed;
extern int D_800F4D74;
extern struct { char pad[8]; short s8, sA; char padC[4]; short s10; } *BadgeRows;
extern struct w3g2e { char pad[0xC]; char c; char pad2[7]; unsigned char b14; char pad3[0x23]; char b38; } *UiRows;
extern struct { char pad[4]; char b4; char pad5[2]; char b7; } *Dialogue;
extern int D_800F4CA8;
extern int AddDrawModePrim();
extern int AddSpritePrim();
extern int rsin();
extern char D_8009F648[];

void func_80032600(void);

void func_80032600(void) {
    int r;
    ModeFrame += 1;
    switch (D_800F4D8A) {
    case 0:
        *((char *)UiRows + MenuCursor * 0x24 + 0xC) = 0;
        StepMenuValue(0, 2, &MenuCursor, 2, 0);
        *((char *)UiRows + MenuCursor * 0x24 + 0xC) = 2;
        if (MenuConfirmPressed != 0) {
            if (UiRows->b14 == 2) {
                PlaySfx(1);
                UiRows->b14 = 3;
                UiRows->b38 = 3;
                BadgeRows->s10 = 4;
                *((char *)UiRows + MenuCursor * 0x24 + 0xC) = 1;
                MenuFadeTimer = 0x40;
                LoadCursor = D_800F4D74;
                ResetScreenCounters();
                if (MenuCursor == 0) {
                    D_800F4D8A = 3;
                } else if (MenuCursor == 1) {
                    D_800F4D8A = 4;
                }
            }
        }
        break;
    case 3:
        if (MenuFadeTimer > 0) MenuFadeTimer = MenuFadeTimer - 1;
        AddScreenFadeTiles(CurrentDrawBuf + 0x70, (0x40 - MenuFadeTimer) * 4, 0x45);
        if (MenuFadeTimer <= 0) {
            r = func_8003B934();
            if (r == 1) {
                ModeGroup = r;
                ModeId = 0x47;
                D_800F4D8A = 0;
            }
        }
        break;
    case 4:
        if (MenuFadeTimer > 0) MenuFadeTimer = MenuFadeTimer - 1;
        AddScreenFadeTiles(CurrentDrawBuf + 0x70, (0x40 - MenuFadeTimer) * 4, 0x45);
        if (MenuFadeTimer <= 0) {
            r = LoadTimListStep(1);
            if (r == 1) {
                ModeGroup = r;
                ModeId = 0xF;
                D_800F4D8A = 0;
            }
        }
        break;
    }
    if (BadgeRows->s10 == 3) {
        Dialogue->b7 = 0;
        Dialogue->b4 = 0x26;
        UpdateDialogue(BadgeRows->s8 + 0xA, BadgeRows->sA + 0x28, Dialogue);
    }
    UpdateBadgeRowAnim(0);
    DrawUiRow(0);
    DrawUiRow(1);
    UpdateScreenFade(0x10084);
}

void InitDialogueState(signed char *a0) {
    PsyqBzero(a0, 0x348);
    a0[5] = -1;
    a0[4] = 0;
    if (D_800F4CA8 == 1) {
        SelectDialogueScript();
    }
}

void DrawDialogueContinueMark(a0, a1)
int a0;
int a1;
{
  int v = rsin((ScreenFrame << 6) & 0xFC0);
  int new_var;
  if (v < 0)
  {
    v += 0x7F;
  }
  new_var = *((int *) SCRATCHPAD);
  *((int *) SCRATCHPAD) = AddDrawModePrim(CurrentDrawBuf + 0x70, AddSpritePrim(CurrentDrawBuf + 0x70, new_var, a0, a1, D_8009F648, 0x1B56, ((v >> 7) + 0x80) * 2), 0x25);
}
