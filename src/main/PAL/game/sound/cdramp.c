/* RODATA 0x80011148 0x14 */
#include "game/sound/cdclose.h"
#include "rr/cd.h"
typedef struct {
    char pad0[0x2670];
    int f2670;
    char pad1[0x2678 - 0x2674];
    int f2678;
    int f267C;
    int f2680;
    char pad2[0x269C - 0x2684];
    int f269C;
    char pad3[0x26A8 - 0x26A0];
    int f26A8;
    int f26AC;
    int f26B0;
    char pad4[0x26C0 - 0x26B4];
    unsigned short h26C0;
} B;

extern struct 
{
  char pad[0x26C6];
  short h26C6;
  unsigned short h26C8;
  unsigned short h26CA;
  unsigned short h26CC;
} SoundState;
extern int CdAudioSetVolume();
extern int CdAudioMute();
extern char CdAudioStatusResult[];
extern int CdAudioSetVolumeRaw();
extern int CdDiskReady();

void CdAudioFadeStep();
void CdAudioPlayStep();

void CdAudioFadeStep()
{
  int d = SoundState.h26C6;
  int t;
  int v;
  int from;
  unsigned c;
  int to;
  if (d > 0)
  {
    c = SoundState.h26C8;
    if (((short) SoundState.h26C8) < d)
    {
      t = c + 1;
      to = SoundState.h26CA;
      from = SoundState.h26CC;
      v = (to - from) << 12;
      v = (v / d) * ((short) t);
      SoundState.h26C8 = t;
      if (v < 0)
      {
        v += 0xFFF;
      }
      v = (v >> 12) + from;
      CdAudioSetVolume(v & 0xFFFF, &SoundState, from);
      return;
    }
    SoundState.h26C6 = 0;
    CdAudioMute(d, &SoundState);
  }
}

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(B (*))&SoundState)

void CdAudioPlayStep()
{
    int r;
    int pad[2];
    B *p;
    switch (CdAudioFadeState) {
    case 0:
        if (CdSync(1, 0) == 0) break;
        if (SoundState.f269C != -1) break;
        SoundState.f2678 = SoundState.f2678 + 1;
    case 1:
        if (CdControl(0x1B, 0, 0)) {
            SoundState.f2678 = SoundState.f2678 + 1;
            break;
        }
        if (CdDiskReady(1) == 0x10) {
            SoundState.f267C = 1;
            SoundState.f2680 = -2;
            SoundState.f2670 = -1;
            SoundState.f2678 = 0;
        }
        break;
    case 2:
        r = CdSync(1, 0);
        if (r == 2) {
            SoundState.f2678 = SoundState.f2678 + 1;
            CdAudioSetVolumeRaw(SoundState.h26C0);
        } else if (r == 5) {
            CdAudioFadeState = 1;
        }
        break;
    case 3:
        CdControl(1, 0, CdAudioStatusResult);
        p = (B *)(CdAudioStatusResult - 0x265C);
        if (*((unsigned char *)p + 0x265C) & 0x20) {
            p->f2678 = p->f2678 + 1;
            break;
        }
        p->f26A8 = p->f26A8 + 1;
        if (p->f26A8 >= 0x169) {
            p->f26B0 = p->f26B0 + 1;
            if (p->f26B0 >= 0x21) p->f26B0 = 0x20;
            p->f2678 = 0;
        }
        break;
    case 4:
        SoundState.f2670 = -1;
        SoundState.f2678 = 0;
        SoundState.f269C = 1;
        SoundState.f2680 = 0;
        SoundState.f26A8 = 0;
        SoundState.f26AC = 0;
        SoundState.f26B0 = 0;
        break;
    }
}

#undef SoundState
