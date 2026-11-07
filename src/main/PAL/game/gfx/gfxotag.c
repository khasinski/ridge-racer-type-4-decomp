#include "types.h"
#include "scratchpad.h"
#include "m2c_macros.h"
#include "game/course/trkintp2.h"
#include "game/demo/opening.h"
#include "game/gfx/gfxdraw.h"
#include "game/sound/sndbgm3.h"
#include "game/sound/sndslot.h"
#include "psyq/libgpu/gpuenv.h"
#include "game/gfx/gfxotag.h"
#include "game/ui/hudmap2.h"
#include "rr/hud_number.h"
#include "rr/bios_devices.h"
#include "rr/race_setup.h"
#include "game/gfx/gfxotag.h"
#include "game/gfx/drawbuf.h"

typedef struct { unsigned addr : 24; unsigned len : 8; } P_TAG;


typedef struct {
    int x, y, z, w;
} V4;

typedef struct {
    short x, y, z, w;
} V8;

typedef struct {
    V4 f0;
    V8 f10;
    int f18;
    int f1C;
    int f20;
    int f24;
} Elem28;

typedef struct {
    char pad0[0x10];
    int f10, f14, f18;
} Arg0Struct;

typedef struct {
    char pad0[0x60];
    V4 f60;
    V8 f70;
    int f78;
    int f7C;
    int f80;
    char pad84[0x20];
    int fA4;
} Context650;

extern struct {
    char pad0[0x50];
    int f50[2];
    int f58[2];
    char pad60[0x55C - 0x60];
    char *f55C;
} RaceContext;
extern short D_800AD520;
extern int ModelBankFormat;
extern short CourseIndex;
extern void SetGteObjectTransform();
extern void SelectModelBank();
extern void SubmitModel();
extern void BuildCameraMatrices();
extern void ApplyMatrix();
extern void SetGeomScreen();
extern void RotMatrixZYX();
extern void ResetCarSlotBuffers();
extern int D_800AC6B0;
extern Elem28 (*D_800ACBA4)[16];
extern short RaceType;
extern M2C_UNK D_800AC6D8;
extern void AddScreenFadeTiles();
extern void SlotsStartForRace();
extern void CdAudioSelectTrack();
extern void CdAudioBeginPlay();
extern void CdAudioFade();
extern void OpeningScriptStep();
extern int PsyqRand();
extern int RaceIntroDone;
extern signed char SelectionState[];
extern signed char Options[];
extern int ViewUsesCarTrackNode;

void func_80044EC4(void);
void ApplyCameraState(int *a);
int InterpCameraKeyframes(Arg0Struct *a0, int a1, int a2);
int func_80045868(char *arg0);

void func_80044EC4(void) {
    int sp10[8];
    int *g = (int *)SCRATCHPAD;
    int s5;
    char *s2;
    int s1;

    s5 = g[0x18];
    s2 = RaceContext.f55C + (D_800AD520 << 7);
    SetupLightMatrices(s2, sp10);
    SetGteObjectTransform((SCRATCHPAD + 0x7C), s2, sp10);
    s1 = RotTransPersOrigin(sp10, s2);
    if (s1 > 0) {
        ClearOTagR(RaceContext.f50[DrawBufIndex], 0x15F);
        g[1] = RaceContext.f50[DrawBufIndex];
        g[0x18] = s1 >> 7;
        if (s1 >> 7 >= 6) {
            g[0x18] = 5;
        }
        SelectModelBank(8);
        SetGteObjectTransform((SCRATCHPAD + 0x7C), s2, sp10);
        g[0x1E] = 0;
        SubmitModel(SCRATCHPAD, ModelBankFormat < 1, 0, 0);
        s1 >>= s5;
        s1 += 0x6E;
        if (s1 > 0) {
            AddShadowDiscPrims(s1 + 1);
            ((P_TAG *)g[1])->addr = ((P_TAG *)&DRAWBUF->ot[s1])->addr;
            ((P_TAG *)&DRAWBUF->ot[s1])->addr = g[1] + 0x578;
        }
    }
    if (D_8009FE60[CourseIndex] == 7) {
        s2 = RaceContext.f55C + (D_800AD520 << 7) + 0x40;
        SetupLightMatrices(s2, sp10);
        SetGteObjectTransform((SCRATCHPAD + 0x7C), s2, sp10);
        s1 = RotTransPersOrigin(sp10, s2);
        if (s1 > 0) {
            ClearOTagR(RaceContext.f58[DrawBufIndex], 0x15F);
            g[1] = RaceContext.f58[DrawBufIndex];
            g[0x18] = s1 >> 7;
            if (s1 >> 7 >= 6) {
                g[0x18] = 5;
            }
            SelectModelBank(9);
            SetGteObjectTransform((SCRATCHPAD + 0x7C), s2, sp10);
            *(int *)(SCRATCHPAD + 0x78) = 0;
            SubmitModel(SCRATCHPAD, ModelBankFormat < 1, 0, 0);
            s1 >>= s5;
            s1 += 0x6E;
            if (s1 > 0) {
                AddShadowDiscPrims(s1 + 1);
                ((P_TAG *)g[1])->addr = ((P_TAG *)&DRAWBUF->ot[s1])->addr;
                ((P_TAG *)&DRAWBUF->ot[s1])->addr = g[1] + 0x578;
            }
        }
    }
    g[0x18] = s5;
    g[1] = (int)CurrentDrawBuf + 0x70;
}

void ApplyCameraState(int *a) {
    int m[8];
    unsigned short v[3];
    short r[3];
    int t[3];
    int *s = (int *)SCRATCHPAD;
    s[6] = -*(short *)((char *)a + 0x10);
    s[7] = *(short *)((char *)a + 0x12);
    s[8] = *(short *)((char *)a + 0x14);
    v[0] = *(short *)((char *)a + 0x10);
    v[1] = *(short *)((char *)a + 0x12);
    v[2] = 0;
    r[0] = 0;
    r[1] = 0;
    r[2] = -*(unsigned short *)((char *)a + 0x1C);
    RotMatrixZYX(v, m);
    ApplyMatrix(m, r, t);
    s[2] = a[0] + t[0];
    s[3] = a[1] + t[1];
    s[4] = a[2] + t[2];
    SetGeomScreen(a[8]);
    BuildCameraMatrices();
}

/* legacy typing of shared globals in this part of the unit */
#define RaceContext (*(Context650 (*))&RaceContext)

int InterpCameraKeyframes(Arg0Struct *a0, int a1, int a2) {
    int pad[4];
    V4 sp20;
    V4 sp30;
    int s0;
    int s3;
    int i;
    Elem28 *s2;
    int t1;
    int t2;

    s0 = -1;
    s2 = D_800ACBA4[a2];
    for (i = 1; i < 0x10; i++) {
        if (s2[i].f24 * 5 >= a1) {
            s0 = i;
            s3 = s0 - 1;
            break;
        }
    }
    if (s0 < 0) {
        return 1;
    }
    if (RaceContext.fA4 != s3) {
        ResetCarSlotBuffers();
        RaceContext.fA4 = s3;
    }
    t1 = (s2[s0].f24 - s2[s3].f24) * 5;
    t2 = a1 - s2[s3].f24 * 5;
    if (s2[s3].f18 == 0) {
        sp20.x = a0->f10;
        sp20.y = a0->f14;
        sp20.z = a0->f18;
    } else {
        sp20 = s2[s3].f0;
    }
    if (s2[s0].f18 == 0) {
        sp30.x = a0->f10;
        sp30.y = a0->f14;
        sp30.z = a0->f18;
    } else {
        sp30 = s2[s0].f0;
    }
    if (t1 > 0) {
        RaceContext.f60.x = ((sp30.x - sp20.x) * t2) / t1 + sp20.x;
        RaceContext.f60.y = ((sp30.y - sp20.y) * t2) / t1 + sp20.y;
        RaceContext.f60.z = ((sp30.z - sp20.z) * t2) / t1 + sp20.z;
        RaceContext.f70.x = ((s2[s0].f10.x - s2[s3].f10.x) * t2) / t1 + s2[s3].f10.x;
        RaceContext.f70.y = ((s2[s0].f10.y - s2[s3].f10.y) * t2) / t1 + s2[s3].f10.y;
        RaceContext.f70.z = ((s2[s0].f10.z - s2[s3].f10.z) * t2) / t1 + s2[s3].f10.z;
        RaceContext.f7C = ((s2[s0].f1C - s2[s3].f1C) * t2) / t1 + s2[s3].f1C;
        RaceContext.f80 = ((s2[s0].f20 - s2[s3].f20) * t2) / t1 + s2[s3].f20;
    } else {
        RaceContext.f60 = sp30;
        RaceContext.f70 = s2[s0].f10;
        RaceContext.f7C = s2[s0].f1C;
        RaceContext.f80 = s2[s0].f20;
    }
    ApplyCameraState(&D_800AC6B0);
    if (RaceType == 1) {
        InitRaceContextBuffers();
        func_80044EC4();
    }
    return 0;
}

#undef RaceContext

void func_80045830(void) {
    PsyqBzero(&D_800AC6D8, 0x20);
    M2C_FIELD(&D_800AC6D8, s32 *, 0x1C) = -1;
}

/* legacy typing of shared globals in this part of the unit */
#define RaceContext (*(int (*)[])&RaceContext)
#define D_800AC6B0 (*(char (*)[])&D_800AC6B0)
#define D_800AC6D8 (*(int (*))&D_800AC6D8)
#define CurrentDrawBuf ((char *)CurrentDrawBuf)

int func_80045868(char *arg0)
{
  int i;
  int s0;
  int *p;
  signed char *q;
  char *r;
  char *pk;
  char *pk2;
  char *ot;
  char *ot2;
  int v;
  int v1;
  int v2;
  int c;
  int *g;
  int *g2;
  int *g1;
  int *gB;
  int *gC;
  int *gL;
  int *gF;
  int *gD;
  int *gS;
  int *gT;
  s0 = 0;
  do
  {
    SlotSetPause(s0, 1);
    s0++;
  }
  while (s0 < 4);
  g1 = RaceContext;
  if ((*((int *) (((char *) g1) + 0x90))) != 0)
  {
    AddScreenFadeTiles(CurrentDrawBuf + 0x70, (*((int *) (((char *) g1) + 0x8C))) * 10, 0x25);
    v1 = (*((int *) (((char *) g1) + 0xA0))) - 8;
    *((int *) (((char *) g1) + 0xA0)) = v1;
    if (v1 < 0)
    {
      *((int *) (((char *) g1) + 0xA0)) = 0;
    }
    SetSoundChannelVolLR(0, (short) (*((int *) (((char *) g1) + 0xA0))), (short) (*((int *) (((char *) g1) + 0xA0))));
    *((int *) (((char *) g1) + 0x8C)) = (*((int *) (((char *) g1) + 0x8C))) + 1;
  }
  else
  {
    v2 = (*((int *) (((char *) g1) + 0xA0))) + 8;
    *((int *) (((char *) g1) + 0xA0)) = v2;
    if (v2 >= 0x61)
    {
      *((int *) (((char *) g1) + 0xA0)) = 0x60;
    }
    SetSoundChannelVolLR(0, (short) (*((int *) (((char *) g1) + 0xA0))), (short) (*((int *) (((char *) g1) + 0xA0))));
  }
  gB = RaceContext;
  s0 = gB[0x88 / 4];
  if (s0 < 0xC)
  {
    if (s0 == 1)
    {
      ResetOpeningFadeTimer();
      if (gB[0x94 / 4] == s0)
      {
        CdAudioBeginPlay(0);
      }
      {
        int two;
        s0 = 0;
        two = 2;
        q = Options;
        p = gB;
        do
        {
          s0++;
          p[0xA8 / 4] = q[0x2A];
          __asm__ volatile("" : "+r"(q) : "r"(p)); /* ASM_BARRIER: prevent CSE of q + 0x2A and prioritize p for a0 */
          q[0x2A] = two;
          q += 4;
          p++;
        }
        while (s0 < 2);
      }
    }
    {
      int x, d;
      x = D_800AC6D8 << 8;
      d = x / 12;
      s0 = 0x100;
      __asm__("" : "+r"(s0) : "r"(x)); /* ASM_BARRIER: keep 0x100 in s0 and prioritize x in v0 */
      s0 -= d;
    }
    OpeningScriptStep(0x1C2);
    {
      int w, low, high;
      low = 0xFFFFFF;
      high = 0xFF000000;
      pk = *((char **) SCRATCHPAD);
      pk[3] = 3;
      pk[7] = 0x62;
      pk[4] = s0;
      pk[5] = s0;
      pk[6] = s0;
      ot = CurrentDrawBuf;
      __asm__("" : "+r"(pk) : "r"(ot), "r"(low)); /* ASM_BARRIER: order ot load before pk load and prioritize low */
      w = *((int *) pk);
      *((short *) (pk + 0xC)) = 0x140;
      *((short *) (pk + 8)) = 0;
      *((short *) (pk + 0xA)) = 0;
      *((short *) (pk + 0xE)) = 0xF0;
      *((int *) pk) = (w & high) | ((*((int *) (ot + 0x70))) & low);
      *((int *) (ot + 0x70)) = ((*((int *) (ot + 0x70))) & high) | (((int) pk) & low);
      pk += 0x10;
      *((char **) SCRATCHPAD) = pk;
      pk[3] = 1;
      ot2 = CurrentDrawBuf;
      __asm__("" : "+r"(pk) : "r"(ot2)); /* ASM_BARRIER: order ot2 load before pk load */
      w = *((int *) pk);
      *((int *) (pk + 4)) = 0xE1000245;
      *((int *) pk) = (w & high) | ((*((int *) (ot2 + 0x70))) & low);
      *((int *) (ot2 + 0x70)) = ((*((int *) (ot2 + 0x70))) & high) | (((int) pk) & low);
      *((char **) SCRATCHPAD) = pk + 8;
    }
  }
  gC = RaceContext;
  if (gC[0x94 / 4] == 0)
  {
    if ((RaceType == 1) && (SelectionState[SelectionState[0x17] + 8] >= 3))
    {
      s0 = 1;
      gL = gC;
      gL[0x98 / 4] = 0;
      r = (((char *) gL[0x554 / 4]) + (D_800AD520 * 0x280)) + 0x28;
      do
      {
        v = *((int *) (r + 0x24));
        if (gL[0x98 / 4] < v)
        {
          gL[0x98 / 4] = v;
        }
        s0++;
        r += 0x28;
      }
      while (s0 < 0x10);
      gF = RaceContext;
      gF[0x94 / 4] = 1;
      gF[0x98 / 4] = gF[0x98 / 4] * 5;
    }
    else
    {
      gC[0x94 / 4] = 2;
    }
  }
  gD = RaceContext;
  switch (gD[0x94 / 4])
  {
    case 1:
      InterpCameraKeyframes(arg0, gD[0x88 / 4], D_800AD520);
      if (Pad0Buttons & 0x800)
    {
      if (gD[0x88 / 4] >= 0x1A)
      {
        if (gD[0x90 / 4] == 0)
        {
          gD[0x98 / 4] = gD[0x88 / 4] + 0x19;
        }
      }
    }
      gS = RaceContext;
      if ((gS[0x98 / 4] - gS[0x88 / 4]) < 0x1A)
    {
      if (gS[0x90 / 4] == 0)
      {
        gS[0x90 / 4] = 1;
        CdAudioFade(0, 8);
      }
    }
      gT = RaceContext;
      gT[0x88 / 4]++;
      if (gT[0x88 / 4] >= gT[0x98 / 4])
    {
      gT[0x88 / 4] = gT[0x98 / 4];
      gT[0x9C / 4] = 1;
    }
      ViewUsesCarTrackNode = 0;
      break;

    case 2:
      gD[0x88 / 4]++;
      if (gD[0x88 / 4] >= 0x49)
    {
      gD[0x9C / 4] = 1;
      *((unsigned short *) (arg0 + 0x1E4)) = (*((unsigned short *) (arg0 + 0x1E4))) | 0x100;
    }
      if (gD[0x9C / 4] == 0)
    {
      *((short *) (((char *) RaceContext) + 0x30)) = 0;
      *((short *) (((char *) RaceContext) + 0x32)) = 0;
      *((short *) (((char *) RaceContext) + 0x34)) = -0x100;
      CarLocalToWorld(arg0, ((char *) RaceContext) + 0x30, ((char *) RaceContext) + 0x60);
      *((short *) (((char *) RaceContext) + 0x74)) = 0;
      *((int *) (((char *) RaceContext) + 0x80)) = 0x1D6;
      *((short *) (((char *) RaceContext) + 0x70)) = (((*((int *) (((char *) RaceContext) + 0x88))) * 0x190) / 73) - 0x280;
      v = (*((short *) (((char *) RaceContext) + 0x72)) = ((-((*((int *) (((char *) RaceContext) + 0x88))) * 0xB00)) / 73) + 0x600);
      *((int *) (((char *) RaceContext) + 0x7C)) = ((-((*((int *) (((char *) RaceContext) + 0x88))) << 9)) / 73) + 0x400;
      if (((short) v) < (-0x400))
      {
        *((short *) (((char *) RaceContext) + 0x72)) = -0x400;
      }
    }
    else
    {
      *((short *) (((char *) RaceContext) + 0x30)) = 0;
      *((short *) (((char *) RaceContext) + 0x32)) = 0;
      *((short *) (((char *) RaceContext) + 0x34)) = -0x100;
      CarLocalToWorld(arg0, ((char *) RaceContext) + 0x30, ((char *) RaceContext) + 0x60);
      *((short *) (((char *) RaceContext) + 0x70)) = -0xF0;
      *((short *) (((char *) RaceContext) + 0x72)) = -0x400;
      *((int *) (((char *) RaceContext) + 0x7C)) = 0x200;
      *((short *) (((char *) RaceContext) + 0x74)) = 0;
      *((int *) (((char *) RaceContext) + 0x80)) = 0x1D6;
    }
      ApplyCameraState(D_800AC6B0);
      g2 = (int *) (D_800AC6B0 - 0x60);
      if (g2[0x88 / 4] >= 0x33)
    {
      g2[0x90 / 4] = 1;
    }
      break;

  }

  g = RaceContext;
  if (g[0x9C / 4] != 0)
  {
    SlotsStartForRace();
    v = g[0x9C / 4];
    DrawSubOtFlag = 1;
    if (v == 1)
    {
      if (g[0x94 / 4] == v)
      {
        if (RaceBgmListIndex == 0)
        {
          CdAudioSelectTrack(0, (PsyqRand() % 14) + 0xB);
        }
        else
        {
          CdAudioSelectTrack(0, BgmNumberToCdTrack[RaceBgmListIndex - 1]);
        }
      }
      RaceIntroDone = 2;
    }
    s0 = 0;
    p = RaceContext;
    q = Options;
    do
    {
      q[0x2A] = *(((unsigned char *) p) + 0xA8);
      p++;
      q += 4;
      s0++;
    }
    while (s0 < 2);
  }
  else
  {
    DrawSubOtFlag = 0;
  }
  return RaceIntroDone;
}

#undef RaceContext
#undef D_800AC6B0
#undef D_800AC6D8
#undef CurrentDrawBuf
