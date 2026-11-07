/* RODATA 0x80011068 0x20 */
#include "types.h"
#include "m2c_macros.h"
#include "game/data/cdfile2.h"
#include "game/course/trkres2.h"
#include "game/data/trkld1.h"
#include "game/gfx/gfxotag.h"
#include "game/course/trkres2.h"
#include "game/data/cdfile1.h"
#include "game/data/cdfile2.h"
#include "rr/course_choices.h"
#include "game/gfx/model.h"
#include "game/sound/sndsys.h"

extern int D_800A02BC;
extern int D_800A029C[];
extern short RaceType;
extern int LoadTimList();
extern int LoadTimImage();
extern int LoadTimList();
extern int SlotSetupBank();
extern struct { int a, b, c, d; } SlotBankSetup;
extern int LoadVab();
extern int D_800A0228[];
extern struct { char pad[0x2858]; char *p[1]; } SoundState;
extern char *VabLoadArgs[2];
extern int D_8010C218;
extern int RaceContext[];
extern int D_800ACBBC;

int LoadCarSpecStep();
void SetRaceLoadCursor(s32 arg0);
s32 GetRaceLoadCursor(void);
int func_8004D7D8();
int func_8004D994();
int LoadSlotBankStep(int a0, int a1);
int LoadComSlotBankStep(int a0);
int LoadSoundBanksStep();
int LoadSfxVabStep(int a0, int a1);
void RaceLoadReset(void);
int LoadVabPairStep(char *a0);
s32 func_8004E184(void);
s32 func_8004E18C(void);
int LoadRaceSoundStep(void);

int LoadCarSpecStep(a0, a1, a2)
int a0;
int a1;
int a2;
{
  int v;
  int t;
  int e;
  e = 0x44;
  v = 0x1E;
  switch (RaceLoadState)
  {
    case 0:
      switch (RaceType)
    {
      case 1:
        D_800A02BC = GetR4FileIndex(2, D_800A029C[a0] + (a2 + v));
        break;

      case 3:
        D_800A02BC = GetR4FileIndex(2, (a0 * 4) + (a2 + e));
        break;

      case 5:
        D_800A02BC = GetR4FileIndex(2, a0 + 0x74);
        break;

      case 6:
        D_800A02BC = GetR4FileIndex(2, (a0 * 2) + 0x64);
        break;

      default:
        RaceLoadState = 0;
        return 1;

    }

    case 1:
      v = CdFileReadStart(D_800A02BC, RaceLoadCursor);
      if (v != 0)
    {
      RaceLoadState++;
    case 2:
      t = CdFileBytesLoaded();
      if (t != 0)
      {
        SetCarSpecTable(RaceLoadCursor);
        v = AlignUp4(t);
        RaceLoadState = 0;
        RaceLoadCursor += v;
        return 1;
      }
      v = 0;
    }
      return v;
    default:
      return 0;
  }
}

void SetRaceLoadCursor(s32 arg0) {
    RaceLoadCursor = arg0;
    RaceLoadState = 0;
}

s32 GetRaceLoadCursor(void) {
    return RaceLoadCursor;
}

/* legacy typing of shared globals in this part of the unit */
#define RaceLoadCursor ((int *)RaceLoadCursor)
#define RaceContext (*(struct { char pad[0x24C]; char *p; char pad2[0x560 - 0x250]; char *r560; } (*))&RaceContext)

int func_8004D7D8(a0, a1) int a0, a1;
{
    int v, t, e;
    e = 0x86;
    switch (RaceLoadState) {
    case 0:
        v = CdFileReadStart(GetR4FileIndex(2, (a0 * 4) + (a1 + e)), RaceLoadCursor);
        if (v == 0) goto out;
        RaceLoadState++;
    case 1:
        t = CdFileBytesLoaded();
        if (t != 0) {
            LoadTimList((char *)RaceLoadCursor + RaceLoadCursor[1]);
            LoadTimImage((char *)RaceLoadCursor + RaceLoadCursor[2]);
            RaceLoadState++;
        }
        return 0;
    case 2:
        v = CdFileReadStart(GetR4FileIndex(2, 0x96), RaceLoadCursor);
        if (v == 0) goto out;
        RaceLoadState++;
    case 3:
        t = CdFileBytesLoaded();
        if (t != 0) {
            RegisterModelBank((char *)RaceLoadCursor + RaceLoadCursor[1], 8);
            RelocateOffsetTable((char *)RaceLoadCursor + RaceLoadCursor[2]);
            RaceContext.r560 = (char *)RaceLoadCursor + RaceLoadCursor[2];
            ((int *)RaceContext.p)[0x138 / 4] = (int)((char *)RaceLoadCursor + RaceLoadCursor[3]);
            v = AlignUp4(t);
            RaceLoadState = 0;
            RaceLoadCursor = (int *)((char *)RaceLoadCursor + v);
            return 1;
        }
        return 0;
    default:
        return 0;
    }
out:
    return v;
}

#undef RaceLoadCursor
#undef RaceContext

int func_8004D994(a0, a1) int a0, a1;
{
    int v, t;
    switch (RaceLoadState) {
    case 0:
        if (RaceType != 1) return 1;
        v = CdFileReadStart(GetR4FileIndex(2, a1 * 2 + 0x7D), RaceLoadCursor);
        if (v == 0) goto out;
        RaceLoadState++;
        goto c1;
    case 2:
        v = CdFileReadStart(GetR4FileIndex(2, a1 * 2 + 0x7C), RaceLoadCursor);
        if (v == 0) goto out;
        RaceLoadState++;
    case 3:
        t = CdFileBytesLoaded();
        if (t != 0) {
            RegisterModelBank(RaceLoadCursor, 8);
            v = AlignUp4(t);
            RaceLoadCursor += v;
            if (D_8009FE60[a0] < 7) {
                RaceLoadState = 0;
                return 1;
            }
            goto inc;
        }
        return 0;
    case 4:
        v = CdFileReadStart(GetR4FileIndex(2, 0x85), RaceLoadCursor);
        if (v == 0) goto out;
        RaceLoadState++;
    case 1:
    case 5:
    c1:
        t = CdFileBytesLoaded();
        if (t != 0) {
            LoadTimList(RaceLoadCursor);
        inc:
            RaceLoadState++;
        }
        return 0;
    case 6:
        v = CdFileReadStart(GetR4FileIndex(2, 0x84), RaceLoadCursor);
        if (v == 0) goto out;
        RaceLoadState++;
    case 7:
        t = CdFileBytesLoaded();
        if (t != 0) {
            RegisterModelBank(RaceLoadCursor, 9);
            v = AlignUp4(t);
            RaceLoadCursor += v;
            RaceLoadState = 0;
            return 1;
        }
        return 0;
    default:
        return 0;
    }
out:
    return v;
}

/* legacy typing of shared globals in this part of the unit */
#define RaceLoadCursor ((char *)RaceLoadCursor)

int LoadSlotBankStep(int a0, int a1) {
    int v;
    if (a0 < 0) return 1;
    switch (RaceLoadSubState) {
    case 0:
        v = CdFileReadStart(GetR4FileIndex(2, a0 + 3, a0), RaceLoadCursor);
        if (v != 0) {
            RaceLoadSubState += 1;
    case 1:
            if (CdFileBytesLoaded() != 0) {
                SlotBankSetup.c = (int)(RaceLoadCursor + *(int *)(RaceLoadCursor + 4));
                SlotBankSetup.a = (int)(RaceLoadCursor + *(int *)(RaceLoadCursor + 8));
                SlotBankSetup.b = (int)(RaceLoadCursor + *(int *)(RaceLoadCursor + 0xC));
                SlotBankSetup.d = a1;
                RaceLoadCursor += AlignUp4(*(int *)(RaceLoadCursor + 0xC));
                RaceLoadSubState += 1;
            }
            v = 0;
        }
        return v;
    case 2:
        SlotSetupBank(&SlotBankSetup);
        RaceLoadSubState = 0;
        return 1;
    default:
        return 0;
    }
}

#undef RaceLoadCursor

/* legacy typing of shared globals in this part of the unit */
#define RaceLoadCursor ((char *)RaceLoadCursor)

int LoadComSlotBankStep(int a0) {
    int v;
    if (a0 < 0) return 1;
    switch (RaceLoadSubState) {
    case 0:
        v = CdFileReadStart(GetR4FileIndex(2, a0 + 0xC), RaceLoadCursor);
        if (v != 0) {
            RaceLoadSubState += 1;
    case 1:
            if (CdFileBytesLoaded() != 0) {
                SlotBankSetup.c = (int)(RaceLoadCursor + *(int *)(RaceLoadCursor + 4));
                SlotBankSetup.a = (int)(RaceLoadCursor + *(int *)(RaceLoadCursor + 8));
                SlotBankSetup.b = (int)(RaceLoadCursor + *(int *)(RaceLoadCursor + 0xC));
                SlotBankSetup.d = 0;
                RaceLoadCursor += AlignUp4(*(int *)(RaceLoadCursor + 0xC));
                RaceLoadSubState += 1;
            }
            v = 0;
        }
        return v;
    case 2:
        ComSlotSetupBank(&SlotBankSetup);
        RaceLoadSubState = 0;
        return 1;
    default:
        return 0;
    }
}

#undef RaceLoadCursor

int LoadSoundBanksStep(a0, a1, a2)
int a0, a1, a2;
{
    switch (RaceLoadState) {
    case 0:
        if (LoadSlotBankStep(a0, 0) == 0) return 0;
        if (a1 >= 0) RaceLoadState = 1;
        else RaceLoadState = 2;
        break;
    case 1:
        if (LoadSlotBankStep(a1, 1)) {
            RaceLoadState = 0;
            return 1;
        }
        break;
    case 2:
        if (LoadComSlotBankStep(a2)) {
            RaceLoadState = 0;
            return 1;
        }
    }
    return 0;
}

/* legacy typing of shared globals in this part of the unit */
#define RaceLoadCursor ((char *)RaceLoadCursor)

int LoadSfxVabStep(int a0, int a1) {
    int s0 = 0;
    int *s2 = D_800A0228 + a0 * 3;
    int v, t;
    char *v1, *v0;
    switch (RaceLoadSubState) {
    case 0:
        v = CdFileReadStart(GetR4FileIndex(2, s2[0]), RaceLoadCursor);
        if (v != 0) {
            RaceLoadSubState += 1;
    case 1:
            if (CdFileBytesLoaded() != 0) {
                if (a1 > 0) s0 += a1;
                v1 = (char *)(s0 * 4 + (int)RaceLoadCursor);
                VabLoadArgs[0] = RaceLoadCursor + *(int *)(v1 + 4);
                v0 = RaceLoadCursor + *(int *)(v1 + 8);
                VabLoadArgs[1] = v0;
                s0 = v0 - RaceLoadCursor;
                LoadVab(VabLoadArgs, s2[1], s2[2], RaceLoadCursor);
                if (a1 > 0) {
                    SoundState.p[D_8010C218] = RaceLoadCursor + *(int *)(RaceLoadCursor + 4);
                }
                s0 = AlignUp4(s0);
                RaceLoadSubState = 0;
                RaceLoadCursor += s0;
                return 1;
            }
            v = 0;
        }
        return v;
    default:
        return 0;
    }
}

#undef RaceLoadCursor

void RaceLoadReset(void) {
    RaceLoadStepIndex = 0;
    RaceLoadState = 0;
    RaceLoadSubState = 0;
    D_800A0218 = 0;
    RaceLoadCursor = LoadCursor;
}

/* legacy typing of shared globals in this part of the unit */
#define RaceLoadCursor ((char *)RaceLoadCursor)
#define VabLoadArgs (*(int (*)[2])&VabLoadArgs)

int LoadVabPairStep(char *a0) {
    int v;
    int *t = D_800A0228;
    switch (RaceLoadStepIndex) {
    case 0:
        v = CdFileReadStart(GetR4FileIndex(2, 0x11), a0);
        if (v != 0) {
            RaceLoadStepIndex += 1;
    case 1:
            if (CdFileBytesLoaded() != 0) {
                RaceLoadStepIndex += 1;
            }
            v = 0;
        }
        return v;
    case 2:
        v = CdFileReadStart(GetR4FileIndex(2, 0x12), RaceLoadCursor);
        if (v != 0) {
            RaceLoadStepIndex += 1;
    case 3:
            if (CdFileBytesLoaded() != 0) {
                VabLoadArgs[0] = (int)a0;
                VabLoadArgs[1] = (int)RaceLoadCursor;
                LoadVab(VabLoadArgs, t[1], t[2]);
                RaceLoadStepIndex = 0;
                return 1;
            }
            v = 0;
        }
        return v;
    default:
        return 0;
    }
}

#undef RaceLoadCursor
#undef VabLoadArgs

s32 func_8004E184(void) {
    return 1;
}

s32 func_8004E18C(void) {
    return 1;
}

int LoadRaceSoundStep(void) {
    switch (RaceLoadStepIndex) {
    case 0:
        if (LoadSfxVabStep(3, 0) != 0) {
            RaceLoadStepIndex += 1;
        }
        return 0;
    case 1:
        if (LoadSfxVabStep(D_800ACBBC, 0) != 0) {
            RaceLoadStepIndex += 1;
        }
        return 0;
    case 2:
        if (LoadSoundBanksStep(RaceContext[0x570 / 4], RaceContext[0x574 / 4], RaceContext[0x578 / 4]) != 0) {
            RaceLoadStepIndex = 0;
            LoadCursor = RaceLoadCursor;
            return 1;
        }
        return 0;
    default:
        return 0;
    }
}
