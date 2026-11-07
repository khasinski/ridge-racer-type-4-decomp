/* RODATA 0x80010740 0x1c */
#include "types.h"
#include "scratchpad.h"
#include "rr/race_types.h"
#include "m2c_macros.h"
#include "rr/lookup.h"
#include "rr/sequence_queue.h"
#include "game/car/carai.h"
#include "game/car/carstate.h"
#include "game/course/trkseek2.h"
#include "game/ui/uipart1.h"
#include "game/car/carsetup.h"
#include "game/race/racesave.h"
#include "game/sound/sndbgm3.h"
#include "game/sound/sndvab.h"
#include "psyq/libgte/gtemath3.h"
#include "game/race/racemai2.h"
#include "rr/steering.h"
#include "game/ui/uipart1.h"

typedef struct 
{
  char p0[0x1A0];
  unsigned short s1A0;
  char p1A2[5];
  unsigned char s1A7;
  char p1A8[0x1B8 - 0x1A8];
  short s1B8;
  char p1BA[0x1D4 - 0x1BA];
  short s1D4;
  char p1D6[2];
  short s1D8;
  char p1DA[0x1E4 - 0x1DA];
  unsigned short s1E4;
  short s1E6;
  short s1E8;
  char p1EA[2];
  short s1EC;
  char p1EE[2];
  unsigned char s1F0[4];
  char p1F4[0x224 - 0x1F4];
  short s224;
  char p226[0x24C - 0x226];
  int s24C;
  char p250[0x28C - 0x250];
  int s28C;
  char p290[4];
  int s294;
  char p298[0x2EC - 0x298];
  int s2EC;
} C;

typedef struct {
    s16 x;
    s16 y;
} Vec2s;

typedef union {
    Vec2s pt;
    s32 word;
} Point2D;

typedef struct {
    s32 pad00[5];
    s32 angles14[4];
    s32 pad24[3];
    s32 dx30;
    s32 dy34;
    s32 pad38[2];
    Point2D pts40[2][9];
} ScratchpadCollision;

typedef struct {
    s32 x;
    s32 y;
    s32 z;
    s32 w;
} Vec4i;

typedef struct {
    s16 x;
    s16 y;
} Corner2s;

typedef struct CarCollision {
    s32 pos00[3];
    s32 pad0C;
    s32 pos10[3];
    s32 pad1C;
    s32 pos20[3];
    u8 pad2C[0x54 - 0x2C];
    s32 rot54;
    u8 pad58[0x88 - 0x58];
    Vec4i corners88[4];
    u8 padC8[0xF4 - 0xC8];
    s32 unkF4;
    u8 padF8[0x110 - 0xF8];
    Corner2s corners110[4];
    s16 unk120;
    s16 unk122;
    u8 pad124[0x18C - 0x124];
    s32 unk18C;
    u8 pad190[0x1A0 - 0x190];
    u16 unk1A0;
    s16 unk1A2;
    u8 unk1A4;
    u8 unk1A5;
    u8 pad1A6[0x1B8 - 0x1A6];
    s16 unk1B8;
    u8 pad1BA[0x1D4 - 0x1BA];
    s16 unk1D4;
    s16 unk1D6;
    s16 unk1D8;
    s16 unk1DA;
    u8 pad1DC[0x1E4 - 0x1DC];
    s16 unk1E4;
    s16 unk1E6;
    s16 unk1E8;
    s16 unk1EA;
    u8 pad1EC[0x224 - 0x1EC];
    s16 unk224;
    u8 pad226[0x24C - 0x226];
    s32 unk24C;
    u8 pad250[0x294 - 0x250];
    s32 unk294;
    u8 pad298[0x320 - 0x298];
} CarCollision;

extern void (*CarUpdateStages[])();
extern unsigned char D_8009F7E0[];
extern char PlayerCars[];
extern short NumCars;
extern unsigned short CourseIndex;
extern C *CarTable[];
extern int rcos();
extern s32 Atan2Table(s32, s32);
extern s32 rsin(s32);
extern s32 ReplayState;
extern s32 TrackLength;
extern s16 RaceType;
extern char *TrackNodes;
extern int RaceVoiceFlags;
extern short ReverseCourse;
extern int NumLaps;

void StepRaceFrame();
unsigned PointInQuad(int a0, int a1, int a2, int a3, int a4);
u32 PointInQuadPtr(void *arg0, M2C_UNK arg1);
void ComputeQuadMidpoints(s32 arg0);
void ResolveCarCollisions(CarCollision *arg0);
void func_80039494(RrRaceVehicle *vehicle);
void PlayTrackEventSfx(RrRaceVehicle *vehicle);
void UpdateRumbleStrip(char *p, int b);
void CheckFinalLapEvent(RrRaceVehicle *vehicle);

void StepRaceFrame(arg0)
unsigned int arg0;
{
  int i;
  int j;
  int s4;
  int s0;
  int s5;
  int s6;
  int new_var;
  C *c;
  int *g;
  g = (int *) (SCRATCHPAD + 0x7C);
  s6 = 0;
  s5 = ((int) (CourseIndex << 16)) >> 17;
  switch (arg0)
  {
    case 0:
    default:
      ReadCarInput(PlayerCars, 0);
      UpdatePlayerCar(PlayerCars, 0);
      break;

    case 1:
    case 6:
      ReadCarInput(PlayerCars, 0);
      UpdatePlayerCar(PlayerCars, 0);
      s6 = 1;
      UpdateComCars();
      break;

    case 2:
      ReadCarInput(PlayerCars, 0);
      ReadCarInput(PlayerCars + 0x320, 1);
      UpdatePlayerCar(PlayerCars, 0);
      UpdatePlayerCar(PlayerCars + 0x320, 1);
      break;

    case 3:
      ReadCarInput(PlayerCars, 0);
      ReadCarInput(PlayerCars + 0x320, 1);
      UpdatePlayerCar(PlayerCars, 0);
      UpdatePlayerCar(PlayerCars + 0x320, 1);
      s6 = 1;
      UpdateComCars();
      break;

    case 4:
      for (i = 0; i < NumCars; i++)
      {
        ReadCarInput(CarTable[i], i);
      }
      for (i = 0; i < NumCars; i++)
      {
        if (CarTable[i]->s1E8 != (-1))
        {
          UpdatePlayerCar(CarTable[i], i);
        }
      }
      break;

    case 5:
      s6 = 1;
      UpdateComCars();
      break;
  }

  for (i = 0; i < 7; i++)
  {
    for (*g = 0; *g < NumCars; (*g)++)
    {
      if (CarTable[*g]->s1E8 != (-1))
      {
        CarUpdateStages[i](CarTable[*g]);
      }
    }
  }
  for (i = 0; i < NumCars; i++)
  {
    if (CarTable[i]->s1E8 != (-1))
    {
      if (CarTable[i]->s1A0 & 7)
      {
        SetCarImpactState(2, CarTable[i]);
        StartCollisionShake(CarTable[i], (short) CarTable[i]->s1A0);
      }
      if (((short) CarTable[i]->s1A0) != 0)
      {
        s4 = ((AngleDiffAbs(0x800 - CarTable[i]->s1EC, CarTable[i]->s1D4) - 0x400) * 3) / 5;
        c = CarTable[i];
        if (c->s1D8 >= 0xA1)
        {
          if (c->s1E4 & 0x10)
          {
            if (c->s1D8 >= 0x2F9)
            {
              c->s224 = 0x80;
              c = CarTable[i];
            }
            s0 = (c->s1D8 * 0x3A) / (c->s28C + 1);
            c->s24C -= 0x1388;
            new_var = rcos(s4, c);
            c = CarTable[i];
            c->s294 = (c->s294 * (0x64 - ((s0 * new_var) >> 12))) / 100;
          }
          else
          {
            s0 = (c->s1D8 * 0x1E) / ((c->s2EC >> 8) + 1);
          }
          new_var = rcos(s4, c);
          c = CarTable[i];
          c->s1D8 = (c->s1D8 * (0x64 - ((s0 * new_var) / 4096))) / 100;
        }
      }
      if (CarTable[i]->s1D8 >= 0x321)
      {
        if (CarTable[i]->s1B8 == 0)
        {
          for (j = 0; j < 4; j++)
          {
            if (CarTable[i]->s1F0[j] != 0)
            {
              CarTable[i]->s1D8 -= D_8009F7E0[s5];
              if (CarTable[i]->s1F0[j] == 2)
              {
                CarTable[i]->s1D8 += D_8009F7E0[s5];
              }
              if (s5 == 2 || CarTable[i]->s1A7 == 0)
              {
                CarTable[i]->s1A7 = 0xA;
              }
            }
          }
        }
      }
    }
  }
  for (i = 7; i < 0xB; i++)
  {
    for (*g = 0; *g < NumCars; (*g)++)
    {
      if (CarTable[*g]->s1E8 != (-1))
      {
        CarUpdateStages[i](CarTable[*g]);
      }
    }
  }
  if (s6 == 1)
  {
    UpdateComCarTrackProjections();
  }
}

unsigned PointInQuad(int a0, int a1, int a2, int a3, int a4) {
    unsigned r = 0;
    if (NormalClip(a0, a1, a4) >= 0) {
        if (NormalClip(a1, a3, a4) >= 0) {
            if (NormalClip(a3, a2, a4) >= 0) {
                r = (unsigned)~NormalClip(a2, a0, a4) >> 31;
            }
        }
    }
    return r;
}

u32 PointInQuadPtr(void *arg0, M2C_UNK arg1) {
    u32 var_v0;

    var_v0 = 0;
    if (NormalClip(M2C_FIELD(arg0, s32 *, 8), M2C_FIELD(arg0, s32 *, 0xC), arg1) >= 0) {
        var_v0 = 0;
        if (NormalClip(M2C_FIELD(arg0, s32 *, 0xC), M2C_FIELD(arg0, s32 *, 4), arg1) >= 0) {
            var_v0 = 0;
            if (NormalClip(M2C_FIELD(arg0, s32 *, 4), M2C_FIELD(arg0, s32 *, 0), arg1) >= 0) {
                var_v0 = (u32) ~NormalClip(M2C_FIELD(arg0, s32 *, 0), M2C_FIELD(arg0, s32 *, 8), arg1) >> 0x1F;
            }
        }
    }
    return var_v0;
}

void ComputeQuadMidpoints(s32 arg0) {
    void *temp_v0;

    temp_v0 = (arg0 * 0x24) + SCRATCH_CAR_WORK;
    M2C_FIELD(temp_v0, s16 *, 0x54) = (s16) ((s32) (M2C_FIELD(temp_v0, s16 *, 0x44) + M2C_FIELD(temp_v0, s16 *, 0x48)) >> 1);
    M2C_FIELD(temp_v0, s16 *, 0x56) = (s16) ((s32) (M2C_FIELD(temp_v0, s16 *, 0x46) + M2C_FIELD(temp_v0, s16 *, 0x4A)) >> 1);
    M2C_FIELD(temp_v0, s16 *, 0x58) = (s16) ((s32) (M2C_FIELD(temp_v0, s16 *, 0x44) + M2C_FIELD(temp_v0, s16 *, 0x4C)) >> 1);
    M2C_FIELD(temp_v0, s16 *, 0x5A) = (s16) ((s32) (M2C_FIELD(temp_v0, s16 *, 0x46) + M2C_FIELD(temp_v0, s16 *, 0x4E)) >> 1);
    M2C_FIELD(temp_v0, s16 *, 0x5C) = (s16) ((s32) (M2C_FIELD(temp_v0, s16 *, 0x48) + M2C_FIELD(temp_v0, s16 *, 0x50)) >> 1);
    M2C_FIELD(temp_v0, s16 *, 0x5E) = (s16) ((s32) (M2C_FIELD(temp_v0, s16 *, 0x4A) + M2C_FIELD(temp_v0, s16 *, 0x52)) >> 1);
    M2C_FIELD(temp_v0, s16 *, 0x60) = (s16) ((s32) (M2C_FIELD(temp_v0, s16 *, 0x4C) + M2C_FIELD(temp_v0, s16 *, 0x50)) >> 1);
    M2C_FIELD(temp_v0, s16 *, 0x62) = (s16) ((s32) (M2C_FIELD(temp_v0, s16 *, 0x4E) + M2C_FIELD(temp_v0, s16 *, 0x52)) >> 1);
    M2C_FIELD(temp_v0, s16 *, 0x40) = (s16) ((s32) (M2C_FIELD(temp_v0, s16 *, 0x44) + M2C_FIELD(temp_v0, s16 *, 0x48) + M2C_FIELD(temp_v0, s16 *, 0x4C) + M2C_FIELD(temp_v0, s16 *, 0x50)) >> 2);
    M2C_FIELD(temp_v0, s16 *, 0x42) = (s16) ((s32) (M2C_FIELD(temp_v0, s16 *, 0x46) + M2C_FIELD(temp_v0, s16 *, 0x4A) + M2C_FIELD(temp_v0, s16 *, 0x4E) + M2C_FIELD(temp_v0, s16 *, 0x52)) >> 2);
}

/* legacy typing of shared globals in this part of the unit */
#define AngleDiffAbs ((s32 (*)(s16, s16))AngleDiffAbs)
#define SetCarImpactState ((void (*)(s32, CarCollision *))SetCarImpactState)
#define rcos ((s32 (*)(s32))rcos)
#define PlayerCars (*(CarCollision (*)[2])&PlayerCars)
#define CarTable (*(CarCollision *(*)[])&CarTable)

void ResolveCarCollisions(CarCollision *arg0) {
    ScratchpadCollision *sp = (ScratchpadCollision *)SCRATCH_CAR_WORK;
    CarCollision *cars[2];
    s32 rx[2];
    s32 ry[2];
    s32 dx[2];
    s32 dy[2];
    s32 unused[4];
    CarCollision *other;
    s32 i;
    s32 j;
    s32 j_xor_1;
    s32 k;
    s32 hit;
    s32 sinVal;
    s32 cosVal;
    s32 ang;
    s32 side;
    s32 arg0_18C;
    s32 arg0_F4;
    s32 other_F4;
    s32 arg0_24;
    s32 offX;
    s32 offZ;

    if (RaceType == 0) {
        return;
    }

    sp->pts40[0][1].pt.x = arg0->corners88[0].x;
    sp->pts40[0][1].pt.y = arg0->corners88[0].z;
    sp->pts40[0][2].pt.x = arg0->corners88[1].x;
    sp->pts40[0][2].pt.y = arg0->corners88[1].z;
    sp->pts40[0][3].pt.x = arg0->corners88[2].x;
    sp->pts40[0][3].pt.y = arg0->corners88[2].z;
    sp->pts40[0][4].pt.x = arg0->corners88[3].x;
    sp->pts40[0][4].pt.y = arg0->corners88[3].z;
    i = 0;
    ComputeQuadMidpoints(0);

    arg0_18C = arg0->unk18C;
    arg0_F4 = arg0->unkF4;
    arg0_24 = arg0->pos20[1];

    for (; i < NumCars; i++) {
        other = CarTable[i];
        if (other != arg0 && other->unk1E8 != -1) {
            s32 diffF4;
            s32 diff18C;
            s32 other_24;

            diff18C = ((other->unk18C + TrackLength) - arg0_18C) % TrackLength;
            other_F4 = other->unkF4;
            diffF4 = other_F4 - arg0_F4;
            other_24 = other->pos20[1];

            if (diffF4 < 0) {
                diffF4 = -diffF4;
            }
            offZ = other_24 - arg0_24;
            if (offZ < 0) {
                offZ = -offZ;
            }

            if ((other->unk1B8 == arg0->unk1B8 || offZ < 0x1A) &&
                diffF4 < 0x64 &&
                (diff18C < 0xC8 || TrackLength - 0xC8 < diff18C) &&
                offZ < 0x3C) {
                {
                    s32 tx;
                    s32 tz;
                    tx = other->pos10[0];
                    tx -= arg0->pos10[0];
                    sp->dx30 = tx;
                    tz = other->pos10[2];
                    tz -= arg0->pos10[2];
                    sp->dy34 = tz;
                    sp->pts40[1][1].pt.x = other->corners88[0].x + sp->dx30;
                    sp->pts40[1][1].pt.y = other->corners88[0].z + sp->dy34;
                    sp->pts40[1][2].pt.x = other->corners88[1].x + sp->dx30;
                    sp->pts40[1][2].pt.y = other->corners88[1].z + sp->dy34;
                    sp->pts40[1][3].pt.x = other->corners88[2].x + sp->dx30;
                    sp->pts40[1][3].pt.y = other->corners88[2].z + sp->dy34;
                    sp->pts40[1][4].pt.x = other->corners88[3].x + sp->dx30;
                    sp->pts40[1][4].pt.y = other->corners88[3].z + sp->dy34;
                    offX = tx;
                    offZ = tz;
                    ComputeQuadMidpoints(1);
                }

                for (j = 0; j < 2; j++) {
                    j_xor_1 = j ^ 1;
                    for (k = 0; k < 9; k++) {
                        hit = PointInQuadPtr(&sp->pts40[j][1], *(s32 *)(sp->pts40[j_xor_1] + k));
                        if (hit > 0) {
                            s32 rz0;

                            if (j == 0) {
                                cars[0] = arg0;
                                cars[1] = other;
                                sinVal = rsin(-cars[0]->rot54);
                                cosVal = rcos(-cars[0]->rot54);
                                ang = (sp->dx30 * cosVal + sp->dy34 * sinVal) / 4096;
                                rz0 = (-sp->dx30 * sinVal + sp->dy34 * cosVal) / 4096;
                                offZ = 0;
                                offX = 0;
                            } else {
                                cars[0] = other;
                                cars[1] = arg0;
                                sinVal = rsin(-cars[0]->rot54);
                                cosVal = rcos(-cars[0]->rot54);
                                ang = -((sp->dx30 + sp->dy34) * cosVal) / 4096;
                                rz0 = -((sp->dy34 - sp->dx30) * sinVal) / 4096;
                            }

                            ang = 0x400 - Atan2Table(ang, rz0);
                            side = 0;
                            if (ang >= -cars[0]->unk122 && ang <= cars[0]->unk122) {
                                if (ang <= -cars[0]->unk120 || ang >= cars[0]->unk120) {
                                    side = 2;
                                }
                            } else {
                                side = 1;
                            }

                            if (j_xor_1 != 0) {
                                ang = -Atan2Table(sp->dx30, sp->dy34) + 0xC00;
                            } else {
                                ang = -Atan2Table(-sp->dx30, -sp->dy34) + 0xC00;
                            }
                            sp->dx30 = rsin(ang) << 4;
                            sp->dy34 = rcos(ang) << 4;

                            dx[1] = (sp->pts40[j_xor_1][k].pt.x - (sp->dx30 >> 6)) - offX;
                            dy[1] = (sp->pts40[j_xor_1][k].pt.y - (sp->dy34 >> 6)) - offZ;
                            dx[0] = sp->pts40[j_xor_1][k].pt.x - offX;
                            dy[0] = sp->pts40[j_xor_1][k].pt.y - offZ;

                            rx[0] = (dx[0] * cosVal + dy[0] * sinVal) >> 12;
                            ry[0] = (-dx[0] * sinVal + dy[0] * cosVal) >> 12;
                            rx[1] = (dx[1] * cosVal + dy[1] * sinVal) >> 12;
                            ry[1] = (-dx[1] * sinVal + dy[1] * cosVal) >> 12;

                            ang = Atan2Table(rx[1] - rx[0], ry[1] - ry[0]) & 0xFFF;
                            sp->angles14[0] = Atan2Table(cars[0]->corners110[1].x - rx[0], cars[0]->corners110[1].y - ry[0]) & 0xFFF;
                            sp->angles14[1] = Atan2Table(cars[0]->corners110[0].x - rx[0], cars[0]->corners110[0].y - ry[0]) & 0xFFF;
                            sp->angles14[2] = Atan2Table(cars[0]->corners110[2].x - rx[0], cars[0]->corners110[2].y - ry[0]) & 0xFFF;
                            sp->angles14[3] = Atan2Table(cars[0]->corners110[3].x - rx[0], cars[0]->corners110[3].y - ry[0]) & 0xFFF;

                            for (dy[0] = 0; dy[0] < 3; dy[0]++) {
                                if (ang >= sp->angles14[dy[0]] && ang <= sp->angles14[dy[0] + 1]) {
                                    break;
                                }
                                if (dy[0] == 2) {
                                    dy[0] = 3;
                                }
                            }

                            dy[1] = dy[0];
                            if (dy[0] & 1) {
                                s32 v1;
                                s32 r0;
                                if (dy[0] == 1) {
                                    dy[0] = 0;
                                }
                                v1 = cars[0]->corners110[dy[0]].x;
                                r0 = rx[0];
                                dx[0] = __builtin_abs(rx[1] - r0);
                                dx[1] = __builtin_abs(v1 - r0);
                            } else {
                                s32 v1;
                                s32 r0;
                                v1 = cars[0]->corners110[dy[0]].y;
                                r0 = ry[0];
                                dx[0] = __builtin_abs(ry[1] - r0);
                                dx[1] = __builtin_abs(v1 - r0);
                            }

                            if (dx[0] != 0 && dx[0] >= dx[1]) {
                                sp->dx30 = (sp->dx30 / dx[0]) * dx[1];
                                sp->dy34 = (sp->dy34 / dx[0]) * dx[1];
                            } else {
                                sp->dx30 >>= 6;
                                sp->dy34 >>= 6;
                            }

                            ang = rcos(AngleDiffAbs(cars[0]->unk1D4, cars[1]->unk1D4));
                            cars[1]->pos00[0] -= sp->dx30;
                            cars[1]->pos00[2] -= sp->dy34;
                            cars[1]->pos10[0] -= sp->dx30 >> 6;
                            cars[1]->pos10[2] -= sp->dy34 >> 6;
                            cars[1]->unk1A0 |= hit << 3;

                            if (side < 2) {
                                SetVehicleShake(cars[0], -sp->dx30, -sp->dy34);
                                SetVehicleShake(cars[1], sp->dx30, sp->dy34);
                                if (cars[side]->unk1E4 & 0x10) {
                                    if (cars[side]->unk1D8 >= 0xF1) {
                                        arg0->unk224 = 0x40;
                                    }
                                    cars[side]->unk24C -= 0x1388;
                                    cars[side]->unk294 = (cars[side]->unk294 * 92) / 100;
                                    cars[side ^ 1]->unk1D8 = (cars[side ^ 1]->unk1D8 + (cars[side]->unk1D8 * ang) / 4096) / 2;
                                    if (cars[side ^ 1]->unk1D8 < 0) {
                                        cars[side ^ 1]->unk1D8 = 0;
                                    }
                                    if (cars[side]->unk1D8 >= 0xF1) {
                                        cars[side]->unk1D8 = (cars[side ^ 1]->unk1D8 * 95) / 100;
                                    }
                                } else {
                                    if (cars[side]->unk1D8 > 0) {
                                        cars[side ^ 1]->unk1D8 = (cars[side ^ 1]->unk1D8 + (cars[side]->unk1D8 * ang) / 4096) / 2;
                                        if (cars[side ^ 1]->unk1D8 < 0) {
                                            cars[side ^ 1]->unk1D8 = 0;
                                        }
                                        if (cars[side]->unk1D8 >= 0xF1 || cars[side ^ 1]->unk1E4 >= 0x10) {
                                            cars[side]->unk1D8 = (-cars[side ^ 1]->unk1D8 * 99) / 100;
                                        }
                                    }
                                }
                            } else {
                                SetVehicleShake(cars[0], -sp->dx30 >> 1, -sp->dy34 >> 1);
                                SetVehicleShake(cars[1], sp->dx30 >> 1, sp->dy34 >> 1);
                            }

                            side &= 1;
                            if (k == 0) {
                                s32 tx;
                                s32 tz;
                                tx = other->pos10[0];
                                tx -= arg0->pos10[0];
                                sp->dx30 = tx;
                                tz = other->pos10[2];
                                tz -= arg0->pos10[2];
                                sp->dy34 = tz;
                                sp->pts40[1][1].pt.x = other->corners88[0].x + sp->dx30;
                                sp->pts40[1][1].pt.y = other->corners88[0].z + sp->dy34;
                                sp->pts40[1][2].pt.x = other->corners88[1].x + sp->dx30;
                                sp->pts40[1][2].pt.y = other->corners88[1].z + sp->dy34;
                                sp->pts40[1][3].pt.x = other->corners88[2].x + sp->dx30;
                                sp->pts40[1][3].pt.y = other->corners88[2].z + sp->dy34;
                                sp->pts40[1][4].pt.x = other->corners88[3].x + sp->dx30;
                                sp->pts40[1][4].pt.y = other->corners88[3].z + sp->dy34;
                                offX = tx;
                                offZ = tz;
                                ComputeQuadMidpoints(1);
                            } else {
                                if (cars[side]->unk1A5 == 0) {
                                    SetCarImpactState(8, cars[side]);
                                }
                                if (cars[side ^ 1]->unk1A5 == 0) {
                                    SetCarImpactState(8, cars[side ^ 1]);
                                }
                                if ((arg0 == &PlayerCars[0] || arg0 == &PlayerCars[1]) && arg0->unk1A5 == 0x1E) {
                                    s16 pan = (other_F4 - arg0_F4) / 2;
                                    s16 vol;
                                    if (arg0->unk1EA != 0) {
                                        pan = -pan;
                                    }
                                    vol = cars[side]->unk1DA - cars[side]->unk1D8;
                                    pan += 0x40;
                                    if (vol >= 0x200) {
                                        vol = 0x1FF;
                                    }
                                    if (ReplayState != 2 && arg0->unk1D8 >= 0) {
                                        PlaySfxVolPan(0x21, (vol >> 3) + 0x3C, pan, -1);
                                    }
                                }
                                break;
                            }
                        }
                    }
                    if (hit != 0) {
                        break;
                    }
                }
            }
        }
    }
}

#undef AngleDiffAbs
#undef SetCarImpactState
#undef rcos
#undef PlayerCars
#undef CarTable

void func_80039494(RrRaceVehicle *vehicle)
{
    LookupTrackEvent(3, vehicle->trackNodeF0);
}

void PlayTrackEventSfx(RrRaceVehicle *vehicle)
{
    SetReverbDepthBoth(LookupTrackEvent(2, vehicle->trackNodeF0));
}

void UpdateRumbleStrip(char *p, int b) {
    int s3;
    int s2;
    int t;
    int a1;
    int s0;
    a1 = LookupTrackEvent(4, *(int *)(p + 0xF0));
    s3 = 0;
    s2 = s3;
    if (a1 >= 0) {
        if (a1 == 0) a1 = 3;
        s0 = *(short *)(p + 0x172);
        if (s0 < 0) {
            s0 += 0x100;
            if (s0 > 0) s0 = 0;
        } else {
            s0 -= 0x100;
            if (s0 < 0) s0 = 0;
        }
        if (s0 != 0) {
            s0 = s0 * *(short *)(p + 0x1D8) / 7665;
            if (s0 >= 0x40) {
                s0 = 0x3F;
            } else if (s0 < -0x3F) {
                s0 = -0x3F;
            }
            t = *(int *)(p + 0x54) - 0x800;
            t += *(short *)(TrackNodes + *(int *)(p + 0xF0) * 0x3C + 0x12);
            p = (char *)(t & 0xFFF);
            if (s0 < 0) {
                if ((a1 & 1) > 0) {
                    s2 = -(s0 + s0 * rcos(p) / 4096);
                    s3 = -(s0 + -s0 * rcos(p) / 4096);
                } else goto l16;
            } else {
l16:
                if (s0 > 0) {
                    if ((a1 & 2) > 0) {
                        s3 = s0 + s0 * rcos(p) / 4096;
                        s2 = s0 + -s0 * rcos(p) / 4096;
                    }
                }
            }
        }
    }
    SetLoopVoiceVolumes(b, s2, s3);
}

void CheckFinalLapEvent(RrRaceVehicle *vehicle)
{
    int distanceLimit;
    int courseLength;
    int flags, node;
    if (!(RaceVoiceFlags & 0x100)) {
        if (ReverseCourse != 1) {
            courseLength = TrackLength;
            distanceLimit = (NumLaps - 1) * courseLength;
            if ((vehicle->second + vehicle->first) >= distanceLimit) {
                node = vehicle->trackNodeF0;
                if (LookupTrackEvent(6, node) == 1) {
                    flags = RaceVoiceFlags & 0x100;
                    RaceVoiceFlags = flags;
                    if (flags == 0) {
                        SoundQueueEnqueue(0x12, 0);
                        RaceVoiceFlags |= 0x100;
                    }
                }
            }
        }
    }
}
