#include "game/system/modeslot.h"
/* RODATA 0x800104c4 0x2c */
#include "rr/ui.h"
#include "game/data/ldscn1.h"
#include "game/gfx/fadectl.h"
#include "game/gfx/fadein.h"
#include "game/gfx/fadesel.h"
#include "game/menu/menutran.h"
#include "game/race/racegrid.h"
#include "game/race/ranking.h"
#include "game/system/modepad.h"
#include "game/menu/menusave.h"
#include "game/system/modeinit.h"
#include "rr/audio_cd_controller.h"
#include "rr/driver_control.h"
#include "rr/frame_environment.h"
#include "game/data/ldscn1.h"
#include "game/gfx/gfxotag.h"
#include "game/system/modeinit.h"
#include "game/data/globals.h"
#include "game/data/selection.h"
#include "game/data/screenframe.h"

extern int SeqMuteVolume();
extern int CdDirectoryBuffer;
extern char StrMainMenu0Log[];
extern short MenuFadeTimer;
extern int SelectionPlayer;
extern short PlayerCourseChoices[];
extern int DrawSubOtFlag;
extern int LoadCursor;
extern short LinkRoleSlot;
extern short ReverseCourse;
extern int MaskBitSetting;
extern unsigned short D_800F3974;
extern short ModeId;
extern short MenuScriptId;
extern short ModeGroup;
extern int DialogueContext;
extern int MenuArenaBase;
extern unsigned short RaceType;
extern short ModeSubState;
extern short LinkState;
extern int D_8010BFB8;
extern int InitBadgeRow();
extern char StrInitMainMenu0[];
extern char LinkEntryState[];
extern char *DialogueState;
extern int VSyncThreshold;
extern short MenuCursor;
extern short SceneId;
extern int ModeFrame;
extern short *BadgeRows;
extern int MenuConfirmPressed;
extern void UpdateDialogue();
extern void ResetScreenCounters();
extern void UpdateBadgeRowAnim();
extern void PlaySfx();
extern void StartAttractDemo();

void TitleModeEnter();
void TitleMenuInit();
void TitleMenuUpdate(void);

void TitleModeEnter() {
    short *a0;
    char *a1;
    int a2;
    SetDispMask(0);
    SeqMuteVolume();
    a2 = 0;
    a1 = SelectionRows;
    a0 = PlayerCourseChoices;
    LinkRoleSlot = -1;
    LinkState = 0;
    MenuFadeTimer = 0;
    SelectionPlayer = 0;
    D_8010BFB8 = 0;
    ReverseCourse = 0;
    do {
        a0[0] = 0;
        a0[1] = 0;
        a1[3] = 0;
        a1 += 4;
        a2++;
        a0 += 2;
    } while (a2 < 4);
    DialogueContext = 2;
    MenuScriptId = 4;
    ModeGroup = 1;
    ModeId = 0x23;
    ModeSubState = 0;
    LoadCursor = CdDirectoryBuffer;
    MenuArenaBase = CdDirectoryBuffer;
    if ((unsigned)(RaceType - 2) < 3U) BackupSaveData(CdDirectoryBuffer, a1, a2);
    RaceType = 0;
    _sio_control(1, 1, 0);
    DrawSubOtFlag = 0;
    MaskBitSetting = 0;
    MainMenuLastIndex = (D_800F3974 & 1) + 7;
    DebugPrintStub(StrMainMenu0Log);
}

void TitleMenuInit()
{
  int i;
  int y;
  int v;
  short c7;
  SetupDrawDispEnvs(0, 0, 0, 3);
  c7 = 7;
  i = 0;
  ModeFrame = 0;
  ScreenFrame = 0;
  VSyncThreshold = 0x80;
  ModeMenuCursorSaved = 0;
  MenuCursor = -1;
  LinkEntryState[3] = 0;
  LinkEntryState[2] = 0;
  LinkEntryState[1] = 0;
  LinkEntryState[0] = 0;
  SceneId = 0x104;
  while (i < (MainMenuLastIndex + 1))
  {
    v = i;
    if (MainMenuLastIndex == c7)
    {
      if (i == MainMenuLastIndex)
      {
        v = 8;
      }
    }
    InitUiRow(i, 0x6C, (i * 0x22) + 0x40, 0x54, 0x20, 0, 0, 1, 0x7F, v);
    i++;
  }

  if (CountProgressFlags() == 0x140)
  {
    AllProgressComplete = 1;
  }
  DialogueState[7] = 2;
  InitBadgeRow(0, 0xC, 0x130, 0x128, 0x58, 0, 0);
  BadgeRows[1] = 1;
  _sio_control(1, 1, 0);
  DebugPrintStub(StrInitMainMenu0);
}

/* legacy typing of shared globals in this part of the unit */
#define ScreenFrame (*(unsigned (*))&ScreenFrame)
#define ModeSubState (*(unsigned short (*))&ModeSubState)
#define BadgeRows ((W7C0_F *)BadgeRows)
#define DialogueState ((unsigned char *)DialogueState)
#define UiRows ((unsigned char *)UiRows)
#define InitBadgeRow ((void (*)())InitBadgeRow)
#define _sio_control ((void (*)())_sio_control)

void TitleMenuUpdate(void)
{
    int s0;
    int v1;
    int a1;
    int s0b;
    int c3, c4;
    W7C0_F *a0;

    ModeFrame = ModeFrame + 1;
    ScreenFrame = ScreenFrame + 1;
    s0 = RunIntroScriptStep();
    switch (ModeSubState) {
    case 0:
    case 10:
        _sio_control(1, 1, 0);
        if (SaveNoticeFlags > 0) {
            UpdateBadgeRowAnim(0);
            if (BadgeRows->s10 == 3) {
                if (MenuConfirmPressed == 0 && ModeSubState != 0 && (signed char)DialogueState[6] != 4)
                    goto call;
                if (ModeSubState != 0xA)
                    goto els;
                PlaySfx(3);
                DialogueState[4] = DialogueState[4] + 1;
                goto c8;
            set:
                DialogueState[4] = v1;
                goto call;
            els:
                ModeSubState = 0xA;
            c8:
                v1 = (signed char)DialogueState[4];
                if (v1 < 4) {
                    c3 = 3;
                    c4 = 4;
                    a1 = SaveNoticeFlags;
                    a0 = BadgeRows;
                    do {
                        if ((a1 >> v1) & 1)
                            goto set;
                        if (v1 == c3)
                            a0->s10 = c4;
                        v1++;
                    } while (v1 < 4);
                }
            call:
                UpdateDialogue(BadgeRows->s8 + 0xA, BadgeRows->sA + 0x28, DialogueState);
                return;
            } else if (BadgeRows->s10 == 0xB) {
                ModeSubState = 1;
                SaveNoticeFlags = 0;
                ScreenFrame = 0;
                return;
            }
        } else {
            ModeSubState = 1;
            ScreenFrame = 0;
            return;
        }
        return;
    case 1:
        _sio_control(1, 1, 0);
        if (Pad0Buttons & 0x800) {
            if (s0 == 0)
                InitSceneFade();
            PlaySfx(1);
            func_8005CA04();
            MenuCursor = 0;
            EnableLinkSioTx();
            ModeSubState = 2;
            InitBadgeRow(0, 0x10, 0x1B6, 0x40, 0x22, 0, 0xF00);
            BadgeRows->s2 = 1;
        }
        if (ScreenFrame >= 0x2EE)
            ModeSubState = 5;
        return;
    case 2:
        EnableLinkSioTx();
        s0b = MenuCursor;
        if (UiRows[0x14] == 2) {
            if (ModeSelectMenuStep() != 0) {
                ModeSubState = 3;
                MenuFadeTimer = 0x40;
                ResetScreenCounters();
                StartSceneFadeIn(ModeMenuCursorSaved);
                BadgeRows->s10 = 4;
            }
        }
        if (s0b != MenuCursor)
            ScreenFrame = 0;
        if (ScreenFrame >= 0xBB8)
            ModeSubState = 5;
        DrawModeSelectRows();
        UpdateBadgeRowAnim(0);
        return;
    case 3:
        if (MenuFadeTimer <= 0) {
            if (LoadSelectedModeFileStep() == 1) {
                ModeSubState = 0;
                ModeGroup = PendingModeId >> 8;
                ModeId = PendingModeId & 0xFF;
            }
        } else {
            MenuFadeTimer = MenuFadeTimer - 1;
        }
        DrawModeSelectRows();
        UpdateBadgeRowAnim(0);
        return;
    case 5:
        _sio_control(1, 1, 0);
        if (AttractCycleCounter & 1) {
            ModeGroup = 2;
            ModeId = 3;
            ModeSubState = 0;
        } else {
            StartAttractDemo(0);
        }
        AttractCycleCounter = AttractCycleCounter + 1;
        if (AttractCycleCounter >= 0x64)
            AttractCycleCounter = 0;
        return;
    }
}

#undef ScreenFrame
#undef ModeSubState
#undef BadgeRows
#undef DialogueState
#undef UiRows
#undef InitBadgeRow
#undef _sio_control
