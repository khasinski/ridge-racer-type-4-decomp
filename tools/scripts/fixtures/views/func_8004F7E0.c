#include "types.h"

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(struct { char pad[4]; s32 x4; char p2[4]; s32 xC; char p3[0x24-0x10]; s16 v[1]; char p4[0x2868-0x26]; s32 x2868; } (*))&SoundState)

#undef SoundState

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(struct { char p0[0x4]; s32 x4; char p1[0x4]; s32 xC; s32 x10; char p2[0x44]; s16 x58; char p3[0x61E]; s16 x678; char p4[0x61E]; s16 xC98; char p5[0x61E]; s16 x12B8; } (*))&SoundState)

#undef SoundState

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(struct { char pad[4]; s32 a; char p[4]; s32 b; s32 c; } (*))&SoundState)

#undef SoundState

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

extern int SsUtAllKeyOff();

extern int SsSetReservedVoice();

extern int SoundReset();

extern s32 CarSoundMuted;

void StopAllSoundAndReset(void) {
    SsUtAllKeyOff(0);
    SsSetReservedVoice(4);
    SoundState.b = 1;
    SoundState.a = 0;
    SoundReset(SoundState.c, 0);
    CarSoundMuted = 0;
}
