/* RODATA 0x80011108 0x10 */
#include "types.h"
#include "game/sound/sndbgm3.h"
#include "game/sound/sndctrl.h"
#include "game/sound/sndown.h"
#include "game/sound/sndrst2.h"
#include "game/sound/sndvab.h"
#include "psyq/libsnd/vmvinit.h"
#include "psyq/libspu/spukeyst.h"
#include "game/sound/sndcmd.h"
#include "rr/channels.h"
#include "rr/race_setup.h"

extern short RaceType;
extern struct 
{
  char pad0[0x2C];
  short h2C;
  char pad2E[0x19D0 - 0x2E];
  int f19D0;
  int f19D4;
  int f19D8;
  int f19DC;
  int f19E0;
  unsigned short h19E4;
  short h19E6;
  short arr[1];
  char pad19EA[0x1A28 - 0x19EA];
  int f1A28;
} SoundState;
extern struct 
{
  char pad[0x31];
  unsigned char b31;
  short h32;
} Options;
extern int SsUtKeyOnV();
extern int SsVabClose();
extern int ResetScreenCopyOverlay();
extern int SetSceneColorParams();
extern int SetSceneOverlayEnabled();
extern int SlotsStartForRace();
extern int MaskBitSetting;
extern struct { char p0[0x1C]; s32 x1C; char p1[0x28]; s32 x48; s32 x4C; char p2[0x38C]; s32 x3DC; } RaceContext;

void SoundQueueUpdate();
void SoundCloseVabsAndReset(void);
void SoundInitForRace(void);

void SoundQueueUpdate()
{
  int s0 = 0;
  unsigned short s1;
  int v1;
  register int v0 asm("$2"); /* ASM_BARRIER: pin v0 so both copies read the sra result */
  int t0;
  int t1;
  unsigned short s2;
  if (RaceType == 1)
  {
    if (SoundState.f1A28 > 0)
    {
      SoundState.f1A28 = SoundState.f1A28 - 1;
    }
  }
  if (SoundState.f19D0 != 0)
  {
    s2 = SoundState.arr[SoundState.f19D8];
    if (SoundState.arr[SoundState.f19D8] >= 0)
    {
      if (SoundState.f19E0 != 0)
      {
        if (SoundState.f19D8 > 0)
        {
          goto b7;
        }
      }
      else
      {
        b7:
        if (SpuGetKeyStatus(0x10, SoundState.f19D8) == 1)
        {
          s0 = -1;
        }

      }
      if (s0 == 0)
      {
        v1 = SoundState.h19E6 * Options.b31;
        s1 = SoundState.h19E4;
        if (v1 < 0)
        {
          v1 += 0xF;
        }
        t1 = v1 >> 4;
        v0 = ((short) s1) * Options.b31;
        s0 = t1;
        if (v0 < 0)
        {
          v0 += 0xF;
        }
        t0 = v0 >> 4;
        s1 = t0;
        if (Options.h32 != 0)
        {
          v0 = (((short) t0) + ((short) t1)) / 2;
          s0 = v0;
          s1 = v0;
        }
        SsUtKeyOnV(4, SoundState.h2C, (short) s2, 0, 0x3C, 0, (short) s1, (short) s0);
        SsUtKeyOnV(5, SoundState.h2C, (short) s2, t1 = 1, 0x3C, 0, (short) s1, (short) s0);
        SoundState.f19D8 = SoundState.f19D8 + t1;
      }
    }
    else
    {
      SoundState.f19D0 = 0;
      SoundState.f19D8 = 0;
      SoundState.f19DC = 0;
    }
  }
}

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(struct { char pad[4]; s32 x4; char p2[4]; s32 xC; char p3[0x24-0x10]; s16 v[1]; char p4[0x2868-0x26]; s32 x2868; } (*))&SoundState)

void SoundCloseVabsAndReset(void) {
    int idx[4] = {1, 4, 5, 6};
    int i;
    SsUtAllKeyOff(0);
    for (i = 0; i < 4; i++) {
        if (SoundState.v[idx[i]] >= 0) {
            SsVabClose(SoundState.v[idx[i]]);
            SoundState.v[idx[i]] = -1;
        }
    }
    SsSetReservedVoice(0x18);
    SoundState.xC = 0;
    SoundMasterVolumeReset();
    ResetPlayerVoices();
    InitLoopVoices(0);
    ResetSoundChannelVolLR();
    SoundQueueClear();
    SoundLoopReset();
    ReverbReset();
    SoundStopAll();
    SsUtAllKeyOff(0);
    SoundState.x2868 = -1;
    SoundState.x4 = 0;
}

#undef SoundState

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(struct { char p0[0x4]; s32 x4; char p1[0x4]; s32 xC; s32 x10; char p2[0x44]; s16 x58; char p3[0x61E]; s16 x678; char p4[0x61E]; s16 xC98; char p5[0x61E]; s16 x12B8; } (*))&SoundState)

void SoundInitForRace(void) {
    SsUtAllKeyOff(0);
    SsSetReservedVoice(4);
    SoundState.xC = 1;
    SoundState.x4 = 0;
    SoundReset(SoundState.x10, 1);
    ResetScreenCopyOverlay();
    MaskBitSetting = 1;
    SetSceneColorParams(0, 0x90, 0x90, 0x90);
    SetSceneOverlayEnabled(RaceContext.x3DC);
    SlotsStartForRace();
    SoundState.x58 = 0x52;
    SoundState.x678 = 0x52;
    SoundState.xC98 = 0x4E;
    SoundState.x12B8 = 0x4E;
    RaceContext.x4C = -1;
    RaceContext.x1C = 1;
    RaceContext.x48 = 0;
}

#undef SoundState

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(struct { char pad[4]; s32 a; char p[4]; s32 b; s32 c; } (*))&SoundState)

void StopAllSoundAndReset(void) {
    SsUtAllKeyOff(0);
    SsSetReservedVoice(4);
    SoundState.b = 1;
    SoundState.a = 0;
    SoundReset(SoundState.c, 0);
    CarSoundMuted = 0;
}

#undef SoundState
