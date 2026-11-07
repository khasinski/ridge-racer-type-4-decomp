#include "types.h"
#include "rr/race_types.h"
#include "rr_asm.h"
#include "m2c_macros.h"
#include "game/car/aipath.h"
#include "game/car/carparam.h"
#include "game/course/trktext.h"
#include "game/race/racemai2.h"
#include "game/sound/sndbgm3.h"
#include "game/car/carsetup.h"
#include "game/race/racemai2.h"
#include "game/car/carparam.h"
#include "rr/bios_devices.h"
#include "rr/controller_hud.h"
#include "game/car/aipath.h"
#include "game/car/aipath2.h"
#include "game/car/carsteer.h"
#include "game/race/racesamp.h"

#define H(p, o) (*(unsigned short *)((char *)(p) + (o)))
#define W(b, o) (*(int *)((char *)(b) + (o)))
#define H(b, o) (*(short *)((char *)(b) + (o)))
#define UH(b, o) (*(unsigned short *)((char *)(b) + (o)))

extern void RotMatrix();
extern char *CarRecordPtrs[];
extern unsigned char SelectionRows[][4];
extern int RaceState;
extern int TrackEventSfxEnabled;
extern int ReplayState;
extern int D_800AC538;
extern char PlayerCars[];
extern short func_80022848(void);
extern void ResetCarSlotBuffers();

void ComputeWheelOffsets(char *a0);
void UpdatePlayerCar(char *a0, int a1);
void StepCarPhysics(s32 arg0);
void func_80022CD4(RrRaceVehicle *arg0);

void ComputeWheelOffsets(char *a0)
{
    unsigned short v[3];
    int m[8];
    int i, n, j;
    char *p, *q;
    v[0] = 0;
    v[1] = H(a0, 0x54);
    v[2] = 0;
    RotMatrix(v, m);
    RR_GTE_SETROT(m);
    v[0] = H(a0, 0x110);
    v[1] = H(a0, 0x8C);
    i = 0;
    v[2] = H(a0, 0x112);
    do {
        RR_GTE_MVMVA_ROT_V0_NOCV(v);
        n = i + 1;
        q = a0 + n * 4;
        v[0] = H(q, 0x110);
        j = i << 4;
        v[1] = H(a0 + j, 0x8C);
        j += 0x88;
        v[2] = H(q, 0x112);
        p = a0 + j;
        RR_GTE_STORE_IR(p);
        i = n;
    } while (i < 4);
}

void InitPlayerCar(a0, a1, a2)
char *a0; int a1; int a2;
{
    char *s3, *s2, *p0;
    short *p2;
    int i, t;
    s2 = a0 + 0x1F4;
    s3 = CarRecordPtrs[a1];
    PsyqBzero(a0, 0x320);
    H(s2, 0x84) = SelectionRows[a1][3] & 1;
    H(a0, 0x1E6) = a1 * 2;
    W(s2, 0x24) = a1;
    UH(a0, 0x1E4) = UH(s2, 0x24) | 0x110;
    W(s2, 0xA4) = 4;
    t = H(s3, 0x1DE);
    RaceState = 2;
    W(s2, 0x94) = t;
    if (t < 0x2710) {
        W(s2, 0x94) = 0x2710;
    }
    p2 = (short *)(s3 + 4);
    W(s2, 0x1C) = (int)(s3 + 0x1C8);
    H(s2, 0x34) = 1;
    H(s2, 0x86) = 1;
    W(s2, 0x18) = (int)s3;
    H(s2, 0x48) = 4;
    H(s2, 0x2A) = 0x64;
    i = 0;
    if (W(s3, 0) >= 0) {
        do {
            p0 = (char *)p2 + 2;
            *p2 = (((H(p0, 0) * 0x36C000) / 1600) << 8) / 100000;
            H(p0, 0x30) = 0;
            if (i == W(s3, 0)) {
                W(s2, 0x98) = (H(p0, 0) * 0x490) / 160;
            }
            i++;
            p2 += 0x1E;
        } while (W(s3, 0) >= i);
    }
    PlaceCarOnGrid(a0, a2, p2);
}

void UpdatePlayerCar(char *a0, int a1)
{
  int pad[18];
  int *pp = pad;
  if (RaceState > 0)
  {
    *((short *) (a0 + 0x1EA)) = func_80022848();
    UpdateCarGear(a0, a1);
    UpdatePlayerSteering(a0, a1);
    UpdateCarEngine(a0);
    if ((a0 == PlayerCars) || (a0 == (PlayerCars + 0x320)))
    {
      if (TrackEventSfxEnabled == 1)
      {
        PlayTrackEventSfx(a0);
      }
      if (ReplayState != 2)
      {
        UpdateRumbleStrip(a0, a1 & 1);
      }
    }
  }
  else
    if (RaceState == 0)
  {
    func_800512C4(a1 & 1, 0, 0);
  }
  if (D_800AD508 < 0)
  {
    if (GetMappedPadButtons(a1 & 1) != 0)
    {
      if (((unsigned) (RaceState - 2)) < 2)
      {
        unsigned short t = (*((unsigned short *) (a0 + 0x1E4))) ^ 0x100;
        D_800AC538 ^= 1;
        *((unsigned short *) (a0 + 0x1E4)) = t;
 do { ResetCarSlotBuffers(t); } while (0);
      }
    }
  }
}

void StepCarPhysics(s32 arg0) {
    UpdateCarSteering();
    IntegrateCarMotion(arg0);
    UpdateCarTrackNode(arg0);
    ComputeWheelOffsets(arg0);
    ConstrainCarToTrack(arg0);
}

void func_80022CD4(RrRaceVehicle *arg0) {
    s32 temp_a2;
    void *temp_a1;

    temp_a1 = (u8 *)arg0 + 0x1F4;
    M2C_FIELD(temp_a1, s32 *, 0x74) = 0;
    temp_a2 = (s32)((u32)M2C_FIELD(temp_a1, s32 *, 0x88) - 0x1964U);
    if (M2C_FIELD(temp_a1, s16 *, 0x86) >= 2) {
        M2C_FIELD(temp_a1, s32 *, 0xA0) = 0;
    }
    if (temp_a2 < 0) {
        M2C_FIELD(temp_a1, s32 *, 0x74) = (s32) (temp_a2 >> 2);
        M2C_FIELD(temp_a1, s32 *, 0xA0) = (s32) ((s32)((u32)M2C_FIELD(temp_a1, s32 *, 0xA0) * 7U) / 10);
        return;
    }
    M2C_FIELD(temp_a1, s32 *, 0x74) = temp_a2;
    M2C_FIELD(temp_a1, s32 *, 0x88) = (s32) ((u32)M2C_FIELD(temp_a1, s32 *, 0x88) - (u32)temp_a2);
    M2C_FIELD(temp_a1, s32 *, 0xA0) = (s32) ((u32)M2C_FIELD(temp_a1, s32 *, 0xA0) - (u32)temp_a2);
}
