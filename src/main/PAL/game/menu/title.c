#include "scratchpad.h"
#include "game/car/car.h"
#include "game/course/trkintp2.h"
#include "game/course/trktext.h"
#include "game/gfx/gfxdraw.h"
#include "game/gfx/scene1.h"
#include "game/sound/cdstop.h"
#include "game/sound/sndslot.h"
#include "game/ui/hudmap2.h"
#include "psyq/libc/printf.h"
#include "game/demo/attract.h"
#include "game/demo/opening.h"
#include "game/menu/title.h"
#include "game/ui/hudmap2.h"
#include "game/course/trktext.h"
#include "rr/frame_environment.h"
#include "rr/color_conversion.h"
#include "rr/driver_control.h"
#include "game/gfx/gfxotag.h"
#include "game/gfx/drawbuf.h"

extern int SetModelTexturePage();
extern int ResetCarSlotBuffers();
extern int AddScreenFadeTiles();
extern int DrawOpponentCar();
extern int DrawCars();
extern int DrawTrackSceneryForPlayer();
extern int StepRaceFrame();
extern int DrawFontString();
extern int SoundSetMasterVolume();
extern int CdAudioSelectTrack();
extern int CdAudioBeginPlay();
extern int CdAudioFade();
extern int func_80055330();
extern int func_8005534C();
extern int SelectCourseBgmTrack();
extern int BuildCameraMatrices();
extern int SetupPlayerView();
extern int SetupViewport();
extern int UpdateTrackObjects();
extern int DrawTrackObjects();
extern char RepeatLabel[];
extern char StrFmtTwoDigitsTitle[];
extern char D_800A1A6C[];
extern short MenuFadeTimer;
extern int D_800ACA2C;
extern int D_800ACB0C;
extern int D_800ACB28;
extern int D_800ACB2C;
extern int D_800F2A64;
extern int MaskBitSetting;
extern short ModeId;
extern int MenuConfirmPressed;
extern int D_800F3A68;
extern int D_800F3A6C;
extern short ModeGroup;
extern short ModeSubState;
extern int ViewUsesCarTrackNode;
extern int CarTable[];
extern Car ComCars[];
extern struct 
{
  char pad[0x28A4];
  int w28a4;
  int w28a8;
} SoundState;
typedef struct 
{
  char pad0[0x18];
  int w18;
  int w1c;
  char pad20[0x2C];
  int w4c;
  char pad50[0x38C];
  int w3dc;
  char pad3E0[0xDC];
  int w4bc;
  int w4c0;
  int w4c4;
  int w4c8;
  int w4cc;
  int w4d0;
  int w4d4;
  int w4d8;
  int w4dc;
  unsigned short h4e0;
  short h4e2;
  char pad4E4[0x4];
  unsigned char b4e8;
  unsigned char b4e9;
  unsigned char b4ea;
  char pad4EB[0x1];
  int w4ec;
  char pad4F0[0x10];
  int w500;
  char pad504[0x2C];
  int w530;
  int w534;
  int w538;
} AC;
extern AC RaceContext;
void AttractDemoRaceUpdate()
{
  char sp18[16];
  int i;
  int old;
  int v;
  int one;
  int vv;
  AC *p;
  int c8;
  Car *tp;
  int *ip;
  if (RaceContext.w4bc == 0)
  {
    SetDispMask(0);
    SetupDrawDispEnvs(0, 0, 0, 1);
    SelectCourseBgmTrack();
    CdAudioBeginPlay(0);
    RaceContext.w4dc = 0xFA;
    for (i = 0; i < 7; i++)
    {
      if (D_8010E270[i & 3] < 2)
      {
        ComCars[i].sub1F4.unk124 = 1;
      }
      else
      {
        ComCars[i].sub1F4.unk124 = 0;
      }
    }

    for (i = 0; i < 4; i++)
    {
      SlotSetPause(i, 1);
    }

    MaskBitSetting = 1;
  }
  else
    if (RaceContext.w4bc > 0)
  {
    if (RaceContext.w4bc == 4)
    {
      SetDispMask(1);
      SetupDrawDispEnvs(0, 0, 0, 0);
    }
  }
  StepRaceFrame(5);
  UpdateTrackObjects(0);
  p = &RaceContext;
  MaskBitSetting = (one = 1);
  p->w4bc = p->w4bc + one;
  p->w4c4 = p->w4c4 + one;
  if (p->w4bc >= 0x1771)
  {
    p->w4bc = 0x1770;
  }
  if (p->w4c8 == 0)
  {
    vv = MenuConfirmPressed;
  }
  else
  {
    vv = D_800F3A6C;
  }
  if (vv != 0)
  {
    if (MenuFadeTimer == 0)
    {
      if (p->w4bc >= 0x1A)
      {
        p->w4bc = 0xBB8;
        p->w4cc = one;
      }
    }
  }
  if (RaceContext.w4c8 == 0)
  {
    if (RaceContext.w4bc == 0xBB8)
    {
      MenuFadeTimer = 0x40;
      SoundSetMasterVolume(0, 0x3C);
    }
  }
  else
    if (RaceContext.w4cc == 1)
  {
    MenuFadeTimer = 0x40;
    SoundSetMasterVolume(0, 0x3C);
    RaceContext.w4cc = 2;
  }
  {
    register int c8_1 asm("a1") = RaceContext.w4c8; /* ASM_BARRIER: pin w4c8 to a1 across switch cases and ResetCarSlotBuffers second argument */
    register int one_v0 asm("v0") = 1; /* ASM_BARRIER: pin constant 1 to v0 for w4c8 comparison */
    if (c8_1 == one_v0)
    {
      if (Pad0Buttons & 0x10)
      {
        RaceContext.w534 = RaceContext.w534 + 1;
        if (RaceContext.w534 >= 4)
        {
          RaceContext.w534 = 0;
        }
        switch (RaceContext.w534)
        {
          case 0:
            RaceContext.w3dc = 0;
            RaceContext.w530 = 0;
            ResetCarSlotBuffers(&RaceContext, c8_1);
            break;

          case 1:
            RaceContext.w3dc = c8_1;
            RaceContext.w530 = 0;
            break;

          case 2:
            RaceContext.w3dc = 0;
            __asm__ volatile("" ::: "memory"); /* ASM_BARRIER: keep w3dc store before w530 store in delay slot */
            RaceContext.w530 = c8_1;
            ResetCarSlotBuffers(&RaceContext, c8_1);
            break;

          case 3:
            RaceContext.w3dc = c8_1;
            RaceContext.w530 = c8_1;
            break;

        }

        func_8005534C(D_800ACA2C);
      }
    if (RaceContext.w3dc == 1)
    {
      if (RaceContext.w530 == 0)
      {
        if (D_800F3A3A & 8)
        {
          RaceContext.h4e0 = RaceContext.h4e0 + 1;
        }
        if (D_800F3A3A & 4)
        {
          RaceContext.h4e0 = RaceContext.h4e0 - 1;
        }
        if (((short) RaceContext.h4e0) < 0)
        {
          RaceContext.h4e0 = 0x167;
        }
        if (!(((short) RaceContext.h4e0) < 0x168))
        {
          RaceContext.h4e0 = 0;
        }
        if (D_800F3A3A & 2)
        {
          RaceContext.h4e2 = 0x800;
        }
        if (D_800F3A3A & 1)
        {
          RaceContext.h4e2 = 0;
        }
        if (RaceContext.h4e2 >= 0x801)
        {
          RaceContext.h4e2 = 0x800;
        }
        if (RaceContext.h4e2 < 0)
        {
          RaceContext.h4e2 = 0x800;
        }
        HsvToRgb(&RaceContext.h4e0, &RaceContext.b4e8);
        func_80055330(0, RaceContext.b4e8, RaceContext.b4e9, RaceContext.b4ea);
      }
    }
    if (RaceContext.w530 == 1)
    {
      if (D_800F3A3A & 4)
      {
        RaceContext.w500 = RaceContext.w500 - 0x10;
      }
      if (D_800F3A3A & 8)
      {
        RaceContext.w500 = RaceContext.w500 + 0x10;
      }
      if (RaceContext.w500 < 0)
      {
        RaceContext.w500 = RaceContext.w500 + 0x1000;
      }
      if (RaceContext.w500 >= 0x1001)
      {
        RaceContext.w500 = RaceContext.w500 - 0x1000;
      }
    }
      if (Pad0Buttons & 0x100)
      {
        RaceContext.w538 = RaceContext.w538 ^ 1;
      }
    }
  }
  SetupViewport(0);
  if ((RaceContext.w4c8 == 0) || (RaceContext.w530 == 0))
  {
    UpdateRaceCamera(2, ComCars + RaceContext.w4c0);
  }
  else
  {
    ViewUsesCarTrackNode = 1;
    ViewCarTrackNode = ComCars[0].wF0;
    SetCarAttachedCamera(ComCars, &RaceContext.w4ec);
  }
  if (RaceContext.w4c4 >= 0x178)
  {
    if (RaceContext.w1c != 0)
    {
      if (RaceContext.w530 == 0)
      {
        v = RaceContext.w4c0;
        RaceContext.w4c0 = v - 2;
        if ((v - 2) < 0)
        {
          RaceContext.w4c0 = v + 5;
        }
        RaceContext.w4c = -1;
        RaceContext.w4c4 = 0;
        UpdateRaceCamera(2, ComCars + RaceContext.w4c0);
      }
    }
  }
  BuildCameraMatrices();
  if (RaceContext.w4bc >= 5)
  {
    if (RaceContext.w4c8 == 0)
    {
      func_800556B4();
    }
    else
    {
      func_80055660();
    }
  }
  if (MenuFadeTimer > 0)
  {
    MenuFadeTimer = MenuFadeTimer - 1;
    AddScreenFadeTiles(CurrentDrawBuf + 0x70, (0x40 - MenuFadeTimer) * 4, 0x45);
    if (MenuFadeTimer == 0)
    {
      if (RaceContext.w4c8 == 0)
      {
        ModeGroup = 1;
        ModeId = 3;
      }
      else
      {
        RaceContext.w18 = 1;
        ModeGroup = 2;
        ModeId = 8;
      }
      ModeSubState = 0;
      MaskBitSetting = 0;
      __asm__ volatile("" ::: "memory"); /* ASM_BARRIER: prevent hoisting argument setup for SoundSetMasterVolume before stores */
      SoundSetMasterVolume(0, 0);
    }
  }
  else
    if (D_800ACB0C < 0x1A)
  {
    AddScreenFadeTiles(CurrentDrawBuf + 0x70, (0x19 - D_800ACB0C) * 8, 0x45);
  }
  {
    register int c8_2 asm("a2") = RaceContext.w4c8; /* ASM_BARRIER: pin w4c8 to a2 across w28a4 clamping and GetCdAudioPosition third argument */
    if (c8_2 == 1)
    {
      if (RaceContext.w538 == 0)
      {
        old = SoundState.w28a4;
        if (Pad0Buttons & 0x8000)
        {
          SoundState.w28a4 = SoundState.w28a4 - 1;
        }
        if (Pad0Buttons & 0x2000)
        {
          SoundState.w28a4 = SoundState.w28a4 + 1;
        }
        if (SoundState.w28a4 <= 0)
        {
          SoundState.w28a4 = SoundState.w28a8 - 1;
        }
        if (SoundState.w28a4 >= SoundState.w28a8)
        {
          SoundState.w28a4 = c8_2;
        }
        if (old != SoundState.w28a4)
        {
          RaceContext.w4d4 = 0x1D;
          if (GetCdAudioPosition(&SoundState, old, c8_2) >= 0)
        {
          if (RaceContext.w4d0 == 0)
          {
            CdAudioFade(0, 0x11);
            RaceContext.w4d0 = 0x19;
          }
        }
      }
      if (RaceContext.w4dc > 0)
      {
        RaceContext.w4dc = RaceContext.w4dc - 1;
      }
      if (RaceContext.w4d4 == 0)
      {
        if (RaceContext.w4dc == 0)
        {
          if (GetCdAudioPosition() < 0)
          {
            if (RaceContext.w4d8 == 0)
            {
              SoundState.w28a4 = SoundState.w28a4 + 1;
              if (SoundState.w28a4 >= SoundState.w28a8)
              {
                SoundState.w28a4 = 1;
              }
            }
            CdAudioFade(0, 0x11);
            RaceContext.w4d0 = 0x19;
            RaceContext.w4d4 = 0x1D;
          }
        }
      }
      RaceContext.w4d0 = RaceContext.w4d0 - 1;
      if (RaceContext.w4d0 < 0)
      {
        RaceContext.w4d0 = 0;
      }
      if (RaceContext.w4d4 > 0)
      {
        RaceContext.w4d4 = RaceContext.w4d4 - 1;
        if (RaceContext.w4d4 == 0)
        {
          CdAudioSelectTrack(0, BgmNumberToCdTrack[RaceBgmListIndex - 1]);
          if (RaceContext.w4d8 == 0)
          {
            CdAudioBeginPlay(0);
          }
          else
          {
            CdAudioBeginPlay(1);
          }
          D_800ACB2C = 0xFA;
        }
      }
      if (D_800F3A68 != 0)
      {
        if (RaceContext.w4d8 == 0)
        {
          RaceContext.w4d8 = 1;
        }
        else
        {
          RaceContext.w4d8 = 0;
        }
      }
      if (D_800ACB28 != 0)
      {
        old = 0xC8;
        DrawFontStringRight(0x131, old, RepeatLabel, 0xFF, 0);
      }
      DrawFontStringRight(0xFA, 0xD4, CdTrackTitleStrings + (BgmNumberToCdTrack[SoundState.w28a4 - 1] << 5), 0xFF, 0);
      AddBrightnessSprite(D_800A1A6C, 0xFC, 0xD4, 0xFF);
      AddBrightnessSprite(D_800A1A6C - 4, 0x105, 0xD4, 0xFF);
      PsyqSprintf(sp18, StrFmtTwoDigitsTitle, SoundState.w28a4);
      DrawFontString(0x112, 0xD4, sp18, 0xFF, 0);
      AddBrightnessSprite(D_800A1A6C - 0xC, 0x121, 0xD4, 0xFF);
        AddBrightnessSprite(D_800A1A6C - 8, 0x12A, 0xD4, 0xFF);
        AddSetTexpagePrim(0);
      }
    }
  }
  SetModelTexturePage(0xA);
  if ((RaceContext.w4c8 == 0) || (RaceContext.w530 == 0))
  {
    DrawCars(6);
  }
  else
  {
    register int i_s1 asm("s1") = 1; /* ASM_BARRIER: pin loop counter to s1 */
    register int *ip_v0 asm("v0") = CarTable; /* ASM_BARRIER: load base address into v0 before adding 4 into s0 */
    register int *ip_s0 asm("s0"); /* ASM_BARRIER: pin array pointer to s0 */
    __asm__ volatile("" : "+r"(ip_v0)); /* ASM_BARRIER: prevent folding +1 into address lo */
    ip_s0 = ip_v0 + 1;
    do
    {
      register int a0 asm("a0") = *ip_s0; /* ASM_BARRIER: load *ip_s0 into a0 first */
      register int a1 asm("a1") = 2; /* ASM_BARRIER: load 2 into a1 before ip_s0 increment */
      __asm__ volatile("" : "+r"(a0), "+r"(a1)); /* ASM_BARRIER: order argument setup before pointer increment */
      ip_s0++;
      __asm__ volatile("" : "+r"(ip_s0)); /* ASM_BARRIER: keep pointer increment before jal so i_s1++ fills delay slot */
      DrawOpponentCar(a0, a1);
      i_s1++;
    }
    while (i_s1 < 7);
  }
  *((int *) (SCRATCHPAD + 0x78)) = D_800F2A64;
  SetupPlayerView(0);
  DrawTrackObjects(0);
  DrawTrackSceneryForPlayer(0);
  QueueDrawStp();
}
