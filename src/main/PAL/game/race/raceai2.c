/* RODATA 0x800106b0 0x1c */
#include "types.h"
#include "scratchpad.h"
#include "rr/player_status.h"
#include "m2c_macros.h"
#include "game/race/racegrid.h"
#include "game/sound/sndcmd.h"
#include "game/system/pad.h"
#include "game/system/pad3.h"
#include "game/ui/hudmap.h"
#include "game/ui/uipart1.h"
#include "rr/bios_events.h"
#include "rr/frame_environment.h"
#include "rr/driver_control.h"
#include "rr/menu_background.h"
#include "rr/audio_loop.h"
#include "game/data/globals.h"
#include "game/gfx/drawbuf.h"

typedef struct { int k; short f; } E;

struct w3g2s { char pad[0x17]; signed char a; char b; signed char c; short d; };

extern int FileWrite();
extern int LinkReadHandle;
extern int LinkWriteHandle;
extern int ClearScreenOverlayState();
extern int VSyncThreshold;
extern int ModeFrame;
extern short ModeId;
extern short ModeGroup;
extern short ModeSubState;
extern short LinkState;
extern int SetMenuFrameOptions();
extern int ExitOnLinkLost();
extern int UpdateLinkHandshake();
extern s16 MenuScriptId;
extern char LinkEntryStateSlot2[];
extern char CourseMenuSelection[];
extern char PlayerCourseChoicesSlot2[];
extern char CourseIndex[];
extern char NumLaps[];
extern char LinkPeerSelectionRows[];
extern char LinkPeerPlayerNames[];
extern short SceneId;
extern short MenuDispMode;
extern int AddTextSprites();
extern int AddDrawModePrim();
extern int AddTilePrim();
extern int AddScreenFadeTiles();
extern char StrLinkError[];
extern char PressStartStr[];
extern short MenuFadeTimer;
extern int MenuConfirmPressed;
extern char StrLinkBattleEnd[];
extern char PressStartText[];
extern int SoundSetMasterVolume();
extern s32 MaskBitSetting;
extern int race_results_init_results();
extern int versus_select_init_results();
extern int time_attack_course_select_init_results();
extern int serial_link_init_results();
extern int extra_course_select_init_results();
extern int SceneAdvanceStep;
extern char ScreenFadeState;
extern int ResetScreenCounters();
extern signed char SelectionState[];
extern E *RankTable[];
extern int CarRecordPtrs;
extern int TotalRaceTime;
extern unsigned char GrandPrixQualifyPlaces[];

int LinkExchange();
void LinkWaitModeEnter(void);
void LinkWaitModeUpdate(void);
void LinkExchangeModeUpdate(void);
void LinkErrorModeUpdate();
void LinkBattleEndModeUpdate(void);
void StopRaceRumble(void);
void StopRaceAudioAndRumble(void);
void RaceExitToMenuStep(void);
void SetupSceneByRaceType(void);
void WaitConfirmModeUpdate(void);
void RecordGrandPrixStageResult(void);
void SortRaceRankTable(void);
void EnterRaceResultScreen(void);
void SortRankAndEnterResultsAlt(void);
void SortRankAndEnterResults(void);
void FinishRace(void);

void DrawLinkEntryScreen(s32 mode) {
    u8 text[4];
    s32 remaining,row,statusY,waitY,labelY;
    s8 status;
    unsigned long address;

    if (mode>=0) {
        /* Integer cursor retains native pointer width. Only the four in-owner
         * addresses are converted back to pointers; the final decrement is dead. */
        row=3;
        address=(unsigned long)&text[3];
        do {
            *(u8*)address=0;
            row--;
            address--;
        } while (row>=0);
        row=0;
        do {
            labelY=row*194+64;
            waitY=row*194+150;
            statusY=row*194+120;
            text[0]=row+((s32)((u32)(LinkRoleSlot^1)<<16)>>15)+49;
            DrawBigFontString(12,labelY,StrPlayerLabel);
            DrawBigFontString(88,labelY,text);
            status=LinkEntryState[1-mode][row];
            switch (status) {
            case 0:
                DrawBigFontString(60,statusY,StrNowAcceptingEntries);
                break;
            case 1:
                DrawBigFontString(60,statusY,EntryAcceptedStr);
                DrawBigFontString(60,waitY,StrStarPleaseWait);
                break;
            case -1:
                DrawBigFontString(60,statusY,StrNoPlayer);
                break;
            }
            row++;
        } while (row<2);
        row=RaceTimeRemaining/50;
        text[0]=row/10+48;
        text[1]=row%10+48;
        DrawBigFontString(44,244,text);
        DrawBigFontString(32,244,SecondsUntilCloseText);
        remaining=(s32)(500U-(u32)RaceTimeRemaining);
        DrawBigFontString(((s32)((u32)remaining<<8)/500)+32,224,StrCountdownMarker);
    }
}

/* legacy typing of shared globals in this part of the unit */
#define LinkRoleSlot (*(short (*))&LinkRoleSlot)

int LinkExchange(a0, a1, a2, a3) int a0, a1, a2, a3;
{
    int r;
    if (LinkRoleSlot != 0) {
        r = -1;
        if (FileWrite(LinkWriteHandle) == a2) {
            if (a3 != 0) {
                if (FileRead(LinkReadHandle, a0, a2) == a2) goto zero;
neg:
                return -1;
            }
            goto zero;
        }
        return r;
    }
    if (FileRead(LinkReadHandle, a1, a2) != a2) goto neg;
    if (a3 != 0) {
        r = -1;
        if (FileWrite(LinkWriteHandle, a0, a2) != a2) return r;
    }
zero:
    return 0;
}

#undef LinkRoleSlot

/* legacy typing of shared globals in this part of the unit */
#define LinkEntryState (*(char (*)[])&LinkEntryState)

void LinkWaitModeEnter(void) {
    unsigned a;
    SetDispMask(0);
    SetupDrawDispEnvs(0, 0, 0, 2);
    LinkEntryState[0] = 1;
    LinkEntryState[1] = 0;
    LinkEntryState[2] = 1;
    LinkEntryState[3] = 0;
    ClearScreenOverlayState();
    RaceTimeRemaining = 0x1F4;
    ModeFrame = 0;
    VSyncThreshold = 0x80;
    ModeSubState = 0;
    LinkState = 0;
    a = ((unsigned)_sio_control(0, 0, 0) >> 7) & 3;
    if (a == 3) {
        ModeGroup = 1;
        ModeId = a;
        ModeSubState = 0;
    } else {
        ModeGroup = 1;
        ModeId = 0x1E;
        ModeSubState = 0;
    }
}

#undef LinkEntryState

/* legacy typing of shared globals in this part of the unit */
#define LinkRoleSlot (*(s16 (*))&LinkRoleSlot)

void LinkWaitModeUpdate(void) {
    ModeFrame += 1;
    if (ExitOnLinkLost() != 1) {
        if (UpdateLinkHandshake(0) != 0) {
            MenuScriptId = 8;
            ModeGroup = 1;
            ModeId = 0x23;
            ModeSubState = 0;
        }
        DrawLinkEntryScreen(LinkRoleSlot);
        AddPartialLine(CurrentDrawBuf + 0x70, 8, 0xF0, 0x138, 0xF0, ModeFrame * 8, 1, 8, 8, 8, 0);
        AddPartialLine(CurrentDrawBuf + 0x70, 8, 0x10C, 0x138, 0x10C, ModeFrame * 8, 1, 8, 8, 8, 0);
        SetMenuFrameOptions(1, 1, 1);
        UpdateScreenFade(0x1003E);
    }
}

#undef LinkRoleSlot

void LinkExchangeModeUpdate(void) {
    int s = LinkState;
    if (s == 1) goto st1;
    if (s < 2) {
        if (s == 0) goto st0;
        goto other;
    }
    if (s == 2) goto st2;
    goto other;
st0:
    SetDispMask(0);
    OpenLinkSerial();
    _sio_control(3, 0, 0);
    _sio_control(5, 0, 0);
    _sio_control(3, 0, 0);
    LinkState = 1;
    return;
st1:
    if (LinkExchange(LinkEntryStateSlot2, LinkEntryStateSlot2 - 2, 2, 1) == -1) { LinkState = 3; return; }
    if (LinkExchange(LinkPeerPlayerNames, LinkPeerPlayerNames - 0x18, 0x18, 1) == -1) { LinkState = 3; return; }
    if (LinkExchange(LinkPeerSelectionRows, LinkPeerSelectionRows - 8, 8, 1) == -1) { LinkState = 3; return; }
    if (LinkExchange(PlayerCourseChoicesSlot2, PlayerCourseChoicesSlot2 - 8, 8, 1) == -1) { LinkState = 3; return; }
    if (LinkExchange(CourseIndex, CourseIndex, 2, 0) == -1) { LinkState = 3; return; }
    if (LinkExchange(CourseMenuSelection, CourseMenuSelection, 2, 0) == -1) { LinkState = 3; return; }
    if (LinkExchange(NumLaps, NumLaps, 4, 0) == -1) { LinkState = 3; return; }
    LinkState = 2;
    CloseLinkSerial();
    SetDispMask(1);
    return;
st2:
    _sio_control(3, 0, 0);
    MenuDispMode = 0;
    MenuScriptId = 0xA;
    SceneId = 0x11A;
    ModeGroup = 1;
    ModeId = 0x23;
    ModeSubState = 0;
    return;
other:
    CloseLinkSerial();
    SetDispMask(1);
    ModeGroup = 1;
    ModeId = 0x2C;
    ModeSubState = 0;
}

/* legacy typing of shared globals in this part of the unit */
#define ModeFrame (*(unsigned int (*))&ModeFrame)
#define ModeId (*(unsigned short (*))&ModeId)
#define ModeSubState (*(unsigned short (*))&ModeSubState)

void LinkErrorModeUpdate()
{
  int s3 = ModeSubState;
  unsigned int new_var;
  int *g;
  switch (s3)
  {
    case 0:
      SetDispMask(0);
      SetupDrawDispEnvs(0, 0, 0, 3);
      VSyncThreshold = 0x80;
      StopRaceAudioAndRumble();
      ModeFrame = 0;
      ModeSubState += 1;
      return;

    case 1:
      SetDispMask(1);
      ModeSubState += 1;
      return;

    case 2:
      ModeFrame += 1;
      _sio_control(1, 1, 0);
      g = (int *) SCRATCHPAD;
      new_var = *g;
      *g = AddDrawModePrim(CurrentDrawBuf + 0x70, AddTilePrim(CurrentDrawBuf + 0x70, AddTextSprites(CurrentDrawBuf + 0x70, AddTextSprites(CurrentDrawBuf + 0x70, new_var, 0x78, 0x100, StrLinkError, 0x7F38, 0, 1), 0x78, 0x114, PressStartStr, 0x7F38, 0, 1), 0, 0, 0x140, 0x200, 0x80, 0, 0), 0x5E);
      if (MenuConfirmPressed == 0)
    {
      if (ModeFrame < 0xFB)
      {
        return;
      }
    }
      ModeSubState = 3;
      MenuFadeTimer = 0x20;
      return;

    case 3:
      if (MenuFadeTimer > 0)
    {
      MenuFadeTimer = ((unsigned short) MenuFadeTimer) - 1;
    }
      AddScreenFadeTiles(CurrentDrawBuf + 0x70, (0x20 - MenuFadeTimer) * 8, 0x45);
      *((int *) SCRATCHPAD) = AddTilePrim(CurrentDrawBuf + 0x74, *((int *) SCRATCHPAD), 0, 0, 0x140, 0x200, (0x20 - MenuFadeTimer) * 4, 0, 0);
      if (MenuFadeTimer <= 0)
    {
      ModeGroup = 1;
      ModeId = s3;
      ModeSubState = 0;
    }
      break;

  }

}

#undef ModeFrame
#undef ModeId
#undef ModeSubState

/* legacy typing of shared globals in this part of the unit */
#define ModeSubState (*(unsigned short (*))&ModeSubState)
#define ModeFrame (*(unsigned int (*))&ModeFrame)

void LinkBattleEndModeUpdate(void)
{
  int s = ModeSubState;
  int new_var;
  switch (s)
  {
    case 0:
      SetDispMask(0);
      SetupDrawDispEnvs(0, 0, 0, 3);
      VSyncThreshold = 0x80;
      ModeFrame = 0;
      ModeSubState++;
      return;

    case 1:
      SetDispMask(1);
      ModeSubState++;
      return;

    case 2:
      ModeFrame++;
      new_var = *((int *) SCRATCHPAD);
      *((int *) SCRATCHPAD) = AddDrawModePrim(CurrentDrawBuf + 0x70, AddTextSprites(CurrentDrawBuf + 0x70, AddTextSprites(CurrentDrawBuf + 0x70, new_var, 0x78, 0x100, StrLinkBattleEnd, 0x7F38, 0, 1), 0x78, 0x114, PressStartText, 0x7F38, 0, 1), 0x3E);
      if (MenuConfirmPressed == 0)
    {
      if (ModeFrame < 0xFB)
      {
        return;
      }
    }
      MenuFadeTimer = 0x20;
      ModeSubState = 3;
      return;

    case 3:
      if (MenuFadeTimer > 0)
    {
      MenuFadeTimer = MenuFadeTimer - 1;
    }
      AddScreenFadeTiles(CurrentDrawBuf + 0x70, (0x20 - MenuFadeTimer) * 8, 0x45);
      if (MenuFadeTimer <= 0)
    {
      ModeGroup = 1;
      ModeId = s;
      ModeSubState = 0;
    }
      break;

  }

}

#undef ModeSubState
#undef ModeFrame

void StopRaceRumble(void) {
    PadRumbleStop(0);
    PadRumbleStop(1);
    PadClearPulse(0);
    PadClearPulse(1);
}

void StopRaceAudioAndRumble(void) {
    SoundSetMasterVolume(0, 0);
    StopRaceRumble();
    MaskBitSetting = 0;
}

/* legacy typing of shared globals in this part of the unit */
#define ModeSubState (*(unsigned short (*))&ModeSubState)

void RaceExitToMenuStep(void) {
    if (ModeSubState == 0) {
        SetDispMask(0);
        StopRaceAudioAndRumble();
        VSyncThreshold = 0x80;
        ModeSubState = 1;
    } else if (ModeSubState == 1) {
        ModeSubState = 2;
    } else if (ModeSubState == 2) {
        MenuScriptId = 6;
        ModeGroup = 1;
        ModeId = 0x23;
        ModeSubState = 0;
    }
    *(int *)SCRATCHPAD = AddTilePrim(CurrentDrawBuf + 0x70, *(int *)SCRATCHPAD, 0, 0, 0x140, 0x100, 8, 8, 8);
}

#undef ModeSubState

void SetupSceneByRaceType(void) {
    SetupDrawDispEnvs(0, 0, 0, 2);
    VSyncThreshold = 0x80;
    ModeFrame = 0;
    SceneAdvanceStep = 0;
    ScreenFadeState = 2;
    SoundCloseVabsAndReset();
    switch (RaceType) {
    case 1:
        race_results_init_results();
        SceneId = 0x130;
        break;
    case 2:
    case 3:
        versus_select_init_results();
        SceneId = 0x13A;
        break;
    case 0:
        time_attack_course_select_init_results();
        SceneId = 0x136;
        break;
    case 4:
        serial_link_init_results();
        SceneId = 0x13E;
        break;
    case 6:
        extra_course_select_init_results();
        SceneId = 0x153;
        break;
    default:
        SceneId = 0x103;
        break;
    }
}

/* legacy typing of shared globals in this part of the unit */
#define ModeFrame (*(u32 (*))&ModeFrame)

void WaitConfirmModeUpdate(void) {
    s32 temp_s0;

    ModeFrame += 1;
    UpdateScreenFade(1);
    if ((u32) ModeFrame < 0x1C2U) {
        if (MenuConfirmPressed != 0) {
            goto block_2;
        }
        goto block_5;
    }
block_2:
    if (SceneAdvanceStep <= 0) {
        SceneAdvanceStep += 1;
        ModeFrame = 0;
        return;
    }
block_5:
    temp_s0 = SceneAdvanceStep;
    if (temp_s0 == 1) {
        if (MenuConfirmPressed != 0) {
            ModeFrame = 0;
            ResetScreenCounters();
            ModeGroup = (s16) temp_s0;
            ModeId = 0xB;
            ModeSubState = 0;
        }
    }
}

#undef ModeFrame

void RecordGrandPrixStageResult(void) {
    int a2;
    int a0;
    int a1;
    a2 = CarRecordPtrs + 0x1E8;
    if (SelectionGpStage >= 0) {
        a1 = 0;
        if (SelectionGpStage >= 2) {
            a1 = 2;
            if (SelectionGpStage >= 4) {
                goto b3;
            }
        }
    } else {
b3:
        a1 = 4;
    }
    a0 = 0;
    do {
        if (RankTable[a0]->f == 0) {
            SelectionState[SelectionState[0x17]] = a0 + 1;
            if (a0 == 0) {
                if (a1 < 4) {
                    if (FramesToMilliseconds(*(int *)(a2 + ((SelectionState[0x17] - a1) << 2)), 0, a2) > TotalRaceTime) {
                        SelectionState[SelectionState[0x17]] = 0;
                    }
                }
            }
            break;
        }
        a0 += 1;
    } while (a0 < 8);
    if (SelectionState[0x19] < 9) {
        SelectionState[0x19] = SelectionState[SelectionState[0x17]];
    }
}

void SortRaceRankTable(void) {
    int i, j;
    int t[2];
    for (i = 0; i < 7; i++) {
        for (j = 7; i < j; j--) {
            t[0] = RankTable[j - 1]->k;
            t[1] = RankTable[j]->k;
            if (t[0] < t[1] || (t[0] == t[1] && RankTable[j]->f == 0)) {
                E *x = RankTable[j - 1];
                E *y = RankTable[j];
                RankTable[j] = x;
                RankTable[j - 1] = y;
            }
        }
    }
    if (RaceType == 1) {
        RecordGrandPrixStageResult();
    }
}

/* legacy typing of shared globals in this part of the unit */
#define SelectionState (*(struct w3g2s (*))&SelectionState)

void EnterRaceResultScreen(void) {
    struct w3g2s *p;
    if (RaceType == 1) {
        p = &SelectionState;
        if (p->c > GrandPrixQualifyPlaces[p->a / 2]) p->d = 5;
        else p->d = 4;
    }
    ModeGroup = 1;
    ModeId = 0x17;
    ModeSubState = 0;
}

#undef SelectionState

void SortRankAndEnterResultsAlt(void) {
    SortRaceRankTable();
    EnterRaceResultScreen();
}

void SortRankAndEnterResults(void) {
    SortRaceRankTable();
    EnterRaceResultScreen();
}

void FinishRace(void) {
    SortRankAndEnterResults();
}
