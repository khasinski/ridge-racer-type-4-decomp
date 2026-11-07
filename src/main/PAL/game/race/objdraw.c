/* RODATA 0x80011364 0x74 */
#include "types.h"
#include "scratchpad.h"
#include "rr_asm.h"
#include "game/car/aipath.h"
#include "game/gfx/gtemodel.h"
#include "game/gfx/poly1.h"
#include "game/gfx/tilerect.h"
#include "game/sound/sndbgm3.h"
#include "game/sound/sndrst2.h"
#include "game/sound/sndvab.h"
#include "psyq/libgte/gtemat3.h"
#include "psyq/libgte/gtemat7.h"
#include "psyq/libgte/gtemath2.h"
#include "game/race/objclr.h"
#include "game/race/objsim.h"
#include "game/race/objdraw.h"
#include "rr/overlay662_distance.h"
#include "rr/overlay661_init.h"
#include "rr/audio_loop.h"
#include "rr/bios_devices.h"

#define SCRATCH_BASE (fp)
#define SCRATCH_PRIM (*(s32 *)(fp + 0x00))
#define SCRATCH_OT   (*(s32 *)(fp + 0x04))
#define SCRATCH_CAMX (*(s32 *)(fp + 0x08))
#define SCRATCH_CAMY (*(s32 *)(fp + 0x0C))
#define SCRATCH_CAMZ (*(s32 *)(fp + 0x10))
#define SCRATCH_ROTX (*(s16 *)(fp + 0x18))
#define SCRATCH_ROTY (*(s16 *)(fp + 0x1C))
#define SCRATCH_ROTZ (*(s16 *)(fp + 0x20))
#define SCRATCH_MAT  ((void *)(fp + 0x28))

typedef struct {
    s16 m[3][3];
    s16 pad;
    s32 t[3];
} Mat32;

typedef struct {
    s16 vx, vy, vz, pad;
} SVec;

typedef struct {
    u8 r, g, b, cd;
} CVec;

typedef struct {
    s32 vx, vy, vz, pad;
} Vec16;

typedef struct {
    SVec v0;
    SVec v1;
    SVec v2;
    SVec pad_v[2];
    u8 pad_28[4];
    CVec c0;
    CVec c1;
    CVec c2;
} Poly38;

typedef struct {
    SVec v0;
    SVec v1;
    SVec v2;
    SVec v3;
    SVec pad_v[3];
    CVec c0;
    CVec c1;
    CVec c2;
    CVec c3;
} Poly48;

typedef struct {
    s32 f00;
    s32 f04;
    s32 f08;
    s32 f0C;
    s32 f10;
    s32 f14;
    s32 f18;
    s32 f1C;
    s32 f20;
    s32 f24;
    s32 f28;
    s32 f2C;
} S_ECB0;

typedef struct {
    s32 f00;
    s32 f04;
    s32 f08;
    s32 f0C;
    s32 f10;
    s32 f14;
    s32 f18;
    SVec rot;
    s32 f24;
    s32 f28;
} S_ECB4;

typedef struct {
    s32 f00;
    s32 f04;
    s32 f08;
    s32 f0C;
    s32 f10;
    s32 f14;
    s32 f18;
    s32 f1C;
    s32 f20;
} S_ECAC;

typedef struct {
    s32 f00[4];
    s16 f10;
    s16 pad[21];
} S_ABEA4;

typedef struct {
    u8 pad00[0x10];
    SVec light_dir[3];
    CVec ambient;
} S_F4D90;

typedef struct { int a[8]; } M;

typedef struct { unsigned short h[16]; } MH;

extern void func_8001A52C(s32);
extern void func_8001ADA0(s32, s32);
extern void func_8005F0D0(void *, s32, s32, s32);
extern void SubmitCourseModel2(void *, s32);
extern void SelectModelBank(s32);
extern void EmitSemiTransG3List(Poly38 *, s32, s32);
extern void EmitSemiTransG4List(Poly48 *, s32, s32);
extern s32 rsin(s32);
extern s32 rcos(s32);
extern void MulMatrix0(Mat32 *, Mat32 *, Mat32 *);
extern void ApplyMatrix(void *, SVec *, Vec16 *);
extern void SetLightMatrix(Mat32 *);
extern void RotMatrix(SVec *, Mat32 *);
extern s32 ratan2(s32, s32);
extern void SetDrawMode(s32, s32, s32, s32, s32);
extern void AddPrim(s32, s32);
extern void PsyqMemset(void *, s32, s32);
extern s32 D_800A3CDC[];
extern s32 D_800A3D18[];
extern SVec CourseLightColors[];
extern CVec CourseColorTable[];
extern S_ABEA4 *TrackNodes;
extern Mat32 LightMatrix;
extern s32 ReplayState;
extern s32 ModeFrame;
extern s16 CourseIndex;
extern s8 RacePaused;
extern S_F4D90 *TrackLightTable;
extern s32 RaceState;
extern s32 D_8010E840;
extern s32 D_8010E848[];
extern Vec16 D_8010E8C8;
extern Vec16 ObjTrailHistory[];
extern S_ECAC *TrackObjectKeyframes;
extern S_ECB0 *TrackObjectDefs;
extern S_ECB4 *TrackObjectStates;
extern int BuildRotMatrixZ();
extern int BuildRotMatrixY();
extern int BuildRotMatrixX();
extern int MulMatrix2();
extern int CameraViewMatrix[];
extern int *D_8010E8D8;
extern int *D_8010E8DC;
extern int D_8010E8E0;

void DrawTrackObjects(s32 arg0);
void BuildObjectMatrix(int unused, short *p, M *out, int flag);
void SetTrackObjectTransform(int a0, int *a1, int a2);
int LoadTrackObjects(int *a, int b);

void DrawTrackObjects(s32 arg0) {
    Mat32 sp18;
    Mat32 sp38;
    Mat32 sp58;
    Mat32 sp78;
    SVec sp98;
    SVec spA0[15];
    SVec sp118;
    SVec sp120;
    CVec sp128[40];
    Vec16 sp1C8;
    Poly38 sp1D8[20];
    Poly48 sp638[80];
    s32 sp1CB8;
    s32 *s3;
    u8 *fp;
    s32 i;
    s32 j;
    s32 k;
    s32 v;
    s32 dist;
    s32 pan;
    s32 q;

    PsyqMemset(&sp98, 0, 8);
    s3 = (s32 *)(SCRATCHPAD + 0x7C);
    fp = (u8 *)SCRATCHPAD;
    for (i = 0; i < TrackObjectCount; i++) {
        if (TrackObjectStates[i].f24 != 1) {
            continue;
        }
        if (arg0 == 2 && (TrackObjectDefs[i].f04 / 10) != 1) {
            continue;
        }

        switch (TrackObjectDefs[i].f04 % 10) {
        case 0:
            BuildObjectMatrix(i, &TrackObjectStates[i].rot, &sp18, arg0);
            SetTrackObjectTransform(i, &sp18, arg0);
            if ((TrackObjectStates[i].f08 / 10) == 1) {
                s3[0x00 / 4] = TrackObjectStates[i].f08 + (CourseIndex % 4) * 3 - 11;
                s3[0x04 / 4] = (TrackObjectStates[i].f00 / 6) % D_800A3CDC[s3[0x00 / 4]];
                s3[0x08 / 4] = D_800A3D18[s3[0x00 / 4] * 12 + s3[0x04 / 4]];
                SubmitCourseModel2(SCRATCH_BASE, TrackObjectDefs[i].f00 + s3[0x08 / 4]);
                continue;
            }
            if ((u32)((TrackObjectStates[i].f04 / 100) - 1) < 2U) {
                SubmitCourseModel2(SCRATCH_BASE, TrackObjectDefs[i].f00 + (TrackObjectStates[i].f04 % 100));
            } else {
                SubmitCourseModel2(SCRATCH_BASE, TrackObjectDefs[i].f00);
            }
            break;

        case 1:
            BuildObjectMatrix(i, &TrackObjectStates[i].rot, &sp18, 0);
            sp38 = LightMatrix;
            if (CourseIndex == 7 || CourseIndex == 3 || CourseIndex == 2) {
                for (j = 0; j < 3; j++) {
                    LightMatrix.m[j][0] = TrackLightTable->light_dir[j].vx;
                    LightMatrix.m[j][1] = -TrackLightTable->light_dir[j].vy;
                    LightMatrix.m[j][2] = TrackLightTable->light_dir[j].vz;
                    sp78.m[0][j] = CourseLightColors[CourseIndex * 3 + j].vx;
                    sp78.m[1][j] = CourseLightColors[CourseIndex * 3 + j].vy;
                    sp78.m[2][j] = CourseLightColors[CourseIndex * 3 + j].vz;
                }
            } else {
                for (j = 0; j < 3; j++) {
                    LightMatrix.m[j][0] = TrackLightTable->light_dir[j].vx;
                    LightMatrix.m[j][1] = TrackLightTable->light_dir[j].vy;
                    LightMatrix.m[j][2] = TrackLightTable->light_dir[j].vz;
                    sp78.m[0][j] = CourseLightColors[CourseIndex * 3 + j].vx;
                    sp78.m[1][j] = CourseLightColors[CourseIndex * 3 + j].vy;
                    sp78.m[2][j] = CourseLightColors[CourseIndex * 3 + j].vz;
                }
            }
            SetBackColor(TrackLightTable->ambient.r, TrackLightTable->ambient.g, TrackLightTable->ambient.b);
            SetColorMatrix(&sp78);
            MulMatrix0(&LightMatrix, &sp18, &sp58);
            SetLightMatrix(&sp58);
            SetTrackObjectTransform(i, &sp18, 0);
            SelectModelBank(4);
            func_8005F0D0(SCRATCH_BASE, TrackObjectDefs[i].f00, 0, 0);
            break;

        case 2:
            func_8001ADA0(TrackObjectDefs[i].f08, TrackObjectDefs[i].f00);
            continue;

        case 3:
            func_8001A52C(TrackObjectDefs[i].f00);
            continue;

        case 4:
            DrawTrackObjectFlashQuads(TrackObjectDefs[i].f00);
            continue;

        case 5:
            if (ReplayState == 2 && RaceType == 1 && D_800AC1A2 < 6) {
                continue;
            }
            if (TrackObjectStates[i].f08 != 0) {
                if (TrackObjectStates[i].f04 + 100 == TrackObjectStates[i].f00) {
                    if (RacePaused == 0 && RaceType != 5) {
                        PlaySfxVolPan(0x27, 0x7F, 0x40, -1);
                    }
                }
                func_80070BA4(SCRATCH_BASE, TrackObjectDefs[i].f00, RacePaused == 0);
                SetDrawMode(SCRATCH_PRIM, 0, 1, 0x2A, 0);
                AddPrim(SCRATCH_OT + 0x900, SCRATCH_PRIM);
                SCRATCH_PRIM += 0xC;
            }
            continue;

        case 6:
            break;
        }

        switch (TrackObjectDefs[i].f08) {
        case 1:
        case 6:
        case 8:
        case 9:
            s3[0x00 / 4] = (TrackObjectStates[i].f0C - SCRATCH_CAMX) >> 4;
            s3[0x04 / 4] = (TrackObjectStates[i].f10 - SCRATCH_CAMY) >> 4;
            s3[0x08 / 4] = (TrackObjectStates[i].f14 - SCRATCH_CAMZ) >> 4;
            dist = CordicSqrt((s3[0x00 / 4] * s3[0x00 / 4] + s3[0x04 / 4] * s3[0x04 / 4] + s3[0x08 / 4] * s3[0x08 / 4]) >> 12);
            if (dist < 0) {
                dist = -dist;
            }
            break;
        }

        switch (TrackObjectDefs[i].f08) {
        case 1:
        case 14:
            sp118.vx = TrackObjectStates[i].rot.vx;
            sp118.vy = (TrackObjectStates[i].f00 * 0x188) % 0x1000;
            sp118.vz = TrackObjectStates[i].rot.vz;
            BuildObjectMatrix(i, &sp118, &sp18, 0);
            MulMatrix0(&sp38, &sp18, &sp58);
            SetLightMatrix(&sp58);
            SetTrackObjectTransform(i, &sp18, 0);
            SelectModelBank(4);
            func_8005F0D0(SCRATCH_BASE, 1, 0, 0);
            if (TrackObjectDefs[i].f08 != 14) {
                v = (0x898 - dist) / 20;
                if (dist >= 0x899) {
                    v = 0;
                }
                s3[0x00 / 4] = D_8010E8C8.vx - SCRATCH_CAMX;
                s3[0x04 / 4] = D_8010E8C8.vy - SCRATCH_CAMY;
                s3[0x08 / 4] = D_8010E8C8.vz - SCRATCH_CAMZ;
                s3[0x10 / 4] = CordicSqrt((s3[0x00 / 4] * s3[0x00 / 4] + s3[0x04 / 4] * s3[0x04 / 4] + s3[0x08 / 4] * s3[0x08 / 4]) >> 10) >> 2;
                if (dist < D_8010E840) {
                    s3[0x10 / 4] = -s3[0x10 / 4];
                }
                {
                    s32 t = ((0x4C8 - (s3[0x10 / 4] / 3)) * 0x1E00) / 1224;
                    if (s3[0x10 / 4] > -800 && s3[0x10 / 4] < 800) {
                        SoundLoopSet(5, t, v, 0x40);
                    }
                }
                D_8010E8C8 = *(Vec16 *)(fp + 8);
                D_8010E840 = dist;
            }
            break;

        case 2:
            sp118.vx = 0;
            sp118.vy = 0x400;
            sp118.vz = 0;
            BuildObjectMatrix(i, &sp118, &sp18, arg0);
            SetTrackObjectTransform(i, &sp18, arg0);
            SubmitCourseModel2(SCRATCH_BASE, 2);
            break;

        case 3:
            sp118.vx = sp118.vy = sp118.vz = 0;
            BuildObjectMatrix(i, &sp118, &sp18, arg0);
            SetTrackObjectTransform(i, &sp18, arg0);
            SubmitCourseModel2(SCRATCH_BASE, 2);
            break;

        case 4:
        case 7:
            sp118.vx = sp118.vy = sp118.vz = 0;
            BuildObjectMatrix(i, &sp118, &sp18, 0);
            SetTrackObjectTransform(i, &sp18, 0);
            if (TrackObjectDefs[i].f08 == 4) {
                for (j = 2; j < 6; j++) {
                    SubmitCourseModel2(SCRATCH_BASE, j);
                }
            } else {
                for (j = 0x2F; j < 0x33; j++) {
                    SubmitCourseModel2(SCRATCH_BASE, j);
                }
            }
            break;

        case 5:
        case 6:
            __asm__ volatile (
                "cfc2 $12, $0; cfc2 $13, $1; ctc2 $12, $8; ctc2 $13, $9; "
                "cfc2 $12, $2; cfc2 $13, $3; ctc2 $12, $10; ctc2 $13, $11; "
                "cfc2 $12, $4; cfc2 $13, $5; ctc2 $12, $12; ctc2 $13, $13; "
                "cfc2 $12, $6; cfc2 $13, $7; ctc2 $12, $14; ctc2 $13, $15"
                : : : "$12", "$13"
            );
            {
                s32 one = 0x1000;
                __asm__ volatile (
                    "ctc2 %0, $0; ctc2 $0, $1; ctc2 %0, $2; ctc2 $0, $3; "
                    "ctc2 %0, $4; ctc2 $0, $5; ctc2 $0, $6; ctc2 $0, $7"
                    : : "r"(one)
                );
            }
            DrawTrackObjectModel(TrackObjectDefs[i].f00 + 1, 1);
            DrawTrackObjectModel(TrackObjectDefs[i].f00 + 2, 0);
            if (TrackObjectDefs[i].f08 != 5) {
                v = (0xDB6 - dist) / 13;
                if (dist >= 0xDB7 || RacePaused != 0) {
                    SoundLoopSet(6, 0x1E00, 0, 0x40);
                    v = 0;
                    if (TrackObjectStates[i].f08 == 1) {
                        TrackObjectStates[i].f08 = 2;
                    }
                }
                if ((v >= 0x74 && TrackObjectStates[i].f08 == 0) || TrackObjectStates[i].f08 == 1) {
                    TrackObjectStates[i].f08 = 1;
                    SoundLoopSet(6, 0x1E00, v, 0x40);
                }
            }
            break;

        case 8:
            v = (0xC80 - dist) / 25;
            if (v >= 0x3D && TrackObjectStates[i].f08 == 0) {
                TrackObjectStates[i].f08 = 1;
                PlaySfxVolPan(0x26, 0x7F, 0x40, -1);
            }
            break;

        case 9:
            v = (0x119E - dist) / 13;
            if (dist >= 0x119F || RacePaused != 0) {
                SoundLoopSet(6, 0x1E00, 0, 0x40);
                v = 0;
                if (TrackObjectStates[i].f08 == 1) {
                    TrackObjectStates[i].f08 = 2;
                }
            }
            if ((v >= 0x22 && TrackObjectStates[i].f08 == 0) || TrackObjectStates[i].f08 == 1) {
                TrackObjectStates[i].f08 = 1;
                SoundLoopSet(6, 0x1E00, v, 0x40);
            }
            break;

        case 11:
            if (RaceState == 0) {
                continue;
            }
            sp18.t[0] = sp18.t[1] = sp18.t[2] = 0;
            sp118.vx = SCRATCH_ROTX;
            sp118.vy = -SCRATCH_ROTY;
            sp118.vz = SCRATCH_ROTZ;
            RotMatrix(&sp118, &sp18);
            RR_GTE_SETROT_TR(&sp18);
            if ((u32)((u16)CourseIndex - 2) < 2U && SCRATCH_CAMX <= 0x21EF7) {
                s3[0x2C / 4] = 2;
            } else {
                s3[0x2C / 4] = 0;
            }
            s3[0x14 / 4] = SCRATCH_CAMX - TrackObjectKeyframes[TrackObjectDefs[i].f24 + s3[0x2C / 4]].f0C;
            s3[0x18 / 4] = SCRATCH_CAMX - TrackObjectKeyframes[TrackObjectDefs[i].f24 + s3[0x2C / 4] + 1].f0C;
            if (s3[0x14 / 4] > 0) {
                sp1C8.vx = s3[0x14 / 4] >> 4;
                v = (0x2A30 - s3[0x14 / 4]) / 100;
            } else if (s3[0x18 / 4] < 0) {
                sp1C8.vx = s3[0x18 / 4] >> 4;
                v = (0x2A30 + s3[0x18 / 4]) / 100;
            } else {
                sp1C8.vx = 0;
                v = 0x6C;
            }
            if (v < 0) {
                v = 0;
            }
            sp1C8.vy = 0;
            sp1C8.vz = (SCRATCH_CAMZ - TrackObjectKeyframes[TrackObjectDefs[i].f24 + s3[0x2C / 4]].f14) >> 4;
            sp120.vx = sp1C8.vx;
            sp120.vy = sp1C8.vy;
            sp120.vz = sp1C8.vz;
            RotTrans(&sp120, &sp1C8, &sp1CB8);
            s3[0x1C / 4] = ratan2(-sp1C8.vz, sp1C8.vx);
            pan = (rcos(s3[0x1C / 4]) / 64) + 0x40;
            if (pan >= 0x80) {
                pan = 0x7F;
            }
            if ((u32)((u16)RaceType - 2) < 3U) {
                if (D_8010E848[i] == ModeFrame) {
                    SetSoundChannelVolLR(1, (s16)((v * pan) >> 7), (s16)((v * (0x80 - pan)) >> 7));
                } else {
                    SetSoundChannelVolLR(0, (s16)((v * pan) >> 7), (s16)((v * (0x80 - pan)) >> 7));
                    SetSoundChannelVolLR(1, 0, 0);
                }
                D_8010E848[i] = ModeFrame;
            } else {
                SetSoundChannelVolLR(0, (s16)((v * pan) >> 7), (s16)((v * (0x80 - pan)) >> 7));
            }
            break;

        case 12:
            if (RaceState == 0) {
                continue;
            }
            s3[0x14 / 4] = SCRATCH_CAMX - TrackObjectKeyframes[TrackObjectDefs[i].f24].f0C;
            pan = 0x40;
            s3[0x18 / 4] = SCRATCH_CAMX - TrackObjectKeyframes[TrackObjectDefs[i].f24 + 1].f0C;
            if (s3[0x14 / 4] > 0) {
                v = (0x2A30 - s3[0x14 / 4]) / 100;
            } else if (s3[0x18 / 4] < 0) {
                v = (0x2A30 + s3[0x18 / 4]) / 100;
            } else {
                v = 0x6C;
            }
            if (v < 0) {
                v = 0;
            }
            if ((u32)((u16)RaceType - 2) < 3U) {
                if (D_8010E848[i] == ModeFrame) {
                    SetSoundChannelVolLR(1, (s16)((v * pan) >> 7), (s16)((v * (0x80 - pan)) >> 7));
                } else {
                    SetSoundChannelVolLR(0, (s16)((v * pan) >> 7), (s16)((v * (0x80 - pan)) >> 7));
                    SetSoundChannelVolLR(1, 0, 0);
                }
                D_8010E848[i] = ModeFrame;
            } else {
                SetSoundChannelVolLR(0, (s16)((v * pan) >> 7), (s16)((v * (0x80 - pan)) >> 7));
            }
            break;

        case 15:
            SetSoundChannelVolLR(0, 0, 0);
            SetSoundChannelVolLR(1, 0, 0);
            break;

        case 10:
        case 13:
            break;
        }

        q = TrackObjectStates[i].f04 / 100;
        if (q == 3) {
            TrackObjectStates[i].f04 = (TrackObjectStates[i].f04 - 300 < 50) ? TrackObjectStates[i].f04 + 10 : TrackObjectStates[i].f04;
        } else if (q == 0) {
            TrackObjectStates[i].f04 = (TrackObjectStates[i].f04 > 0) ? TrackObjectStates[i].f04 - 5 : TrackObjectStates[i].f04;
        }

        if ((TrackObjectStates[i].f04 % 100) == 0) {
            continue;
        }

        switch (TrackObjectDefs[i].f0C % 10) {
        case 1:
            if (arg0 == 2) {
                s3[0x20 / 4] = -1;
            } else {
                s3[0x20 / 4] = 1;
            }
            sp1CB8 = FindTrackNodeAtXZ(TrackObjectStates[i].f0C, TrackObjectStates[i].f14);
            for (j = 0; j < 8; j++) {
                spA0[j].vy = (TrackNodes[sp1CB8].f10 - TrackObjectStates[i].f10) >> 2;
                spA0[j].vx = (((spA0[j].vy >> 1)) * rcos(TrackObjectStates[i].f00 * 57 + j * 512)) / 4096;
                spA0[j].vz = (((spA0[j].vy >> 1)) * rsin(TrackObjectStates[i].f00 * 57 + j * 512)) / 4096 + (((spA0[j].vy >> 1)) + 250) * s3[0x20 / 4];
                sp128[j].b = 0;
                sp128[j].g = 0;
                sp128[j].r = 0;
            }
            {
                k = (PsyqRand() % 10) + (TrackObjectStates[i].f04 % 100);
                if (RacePaused != 0) {
                    k = TrackObjectStates[i].f04 % 100;
                }
                for (j = 0; j < 4; j++) {
                    sp1D8[j].v0.vy = 20;
                    sp1D8[j].v0.vx = 0;
                    sp1D8[j].v0.vz = s3[0x20 / 4] * 50;
                    sp1D8[j].c0.r = sp1D8[j].c0.g = k;
                    sp1D8[j].c0.b = (k * 3) / 4;
                    sp1D8[j].v1.vx = (spA0[j].vx >> 1);
                    sp1D8[j].v1.vy = (spA0[j].vy >> 1);
                    sp1D8[j].v1.vz = (spA0[j].vz >> 1);
                    sp1D8[j].v2.vx = (spA0[(j + 4) % 8].vx >> 1);
                    sp1D8[j].v2.vy = (spA0[(j + 4) % 8].vy >> 1);
                    sp1D8[j].v2.vz = (spA0[(j + 4) % 8].vz >> 1);
                    sp1D8[j].c1 = sp128[j];
                    sp1D8[j].c2 = sp128[(j + 4) % 8];
                }
                for (j = 0; j < 8; j++) {
                    sp1D8[j + 4].v0.vx = 0;
                    sp1D8[j + 4].v0.vy = (TrackNodes[sp1CB8].f10 - TrackObjectStates[i].f10) >> 2;
                    sp1D8[j + 4].v0.vz = ((spA0[j].vy / 4) + 250) * s3[0x20 / 4];
                    sp1D8[j + 4].v1.vx = (spA0[j].vx >> 1);
                    sp1D8[j + 4].v1.vy = spA0[j].vy;
                    sp1D8[j + 4].v1.vz = ((spA0[j].vz >> 1)) + 180;
                    sp1D8[j + 4].v2.vx = (spA0[(j + 1) % 8].vx >> 1);
                    sp1D8[j + 4].v2.vy = spA0[(j + 1) % 8].vy;
                    sp1D8[j + 4].v2.vz = ((spA0[(j + 1) % 8].vz >> 1)) + 180;
                    sp1D8[j + 4].c0.r = sp1D8[j + 4].c0.g = k + 5;
                    sp1D8[j + 4].c0.b = (k * 3) / 4;
                    sp1D8[j + 4].c1.r = sp1D8[j + 4].c1.g = (k + 5) >> 1;
                    sp1D8[j + 4].c1.b = (k * 3) >> 3;
                    sp1D8[j + 4].c2.r = sp1D8[j + 4].c2.g = (k + 5) >> 1;
                    sp1D8[j + 4].c2.b = (k * 3) >> 3;
                }
                BuildObjectMatrix(i, &TrackObjectStates[i].rot, &sp18, arg0);
                SetRotMatrix(&sp18);
                EmitSemiTransG3List(sp1D8, 12, 1);
                for (j = 0; j < 8; j++) {
                    sp638[j].v0.vx = (spA0[j].vx >> 1);
                    sp638[j].v0.vy = spA0[j].vy;
                    sp638[j].v0.vz = ((spA0[j].vz >> 1)) + 180;
                    sp638[j].v1.vx = (spA0[(j + 1) % 8].vx >> 1);
                    sp638[j].v1.vy = spA0[(j + 1) % 8].vy;
                    sp638[j].v1.vz = ((spA0[(j + 1) % 8].vz >> 1)) + 180;
                    sp638[j].v2.vx = spA0[j].vx;
                    sp638[j].v2.vy = spA0[j].vy;
                    sp638[j].v2.vz = spA0[j].vz;
                    sp638[j].v3.vx = spA0[(j + 1) % 8].vx;
                    sp638[j].v3.vy = spA0[(j + 1) % 8].vy;
                    sp638[j].v3.vz = spA0[(j + 1) % 8].vz;
                    sp638[j].c0.r = sp638[j].c0.g = (k + 5) >> 1;
                    sp638[j].c0.b = (k * 3) >> 3;
                    sp638[j].c1.r = sp638[j].c1.g = (k + 5) >> 1;
                    sp638[j].c1.b = (k * 3) >> 3;
                    sp638[j].c2.r = sp638[j].c2.g = sp638[j].c2.b = 0;
                    sp638[j].c3.r = sp638[j].c3.g = sp638[j].c3.b = 0;
                }
                EmitSemiTransG4List(sp638, 8, 1);
            }
            break;

        case 2:
        case 5:
            s3[0x28 / 4] = TrackObjectDefs[i].f0C / 10;
            for (k = 0; k < 18; k++) {
                if (s3[0x28 / 4] == 0) {
                    if (CourseIndex == 0) {
                        sp128[k].r = (k < 9) ? 20 : 20 - (k - 8) * 2;
                        sp128[k].g = (k < 9) ? 20 : 20 - (k - 8) * 2;
                        sp128[k].b = (k < 9) ? 10 : 18 - k;
                    } else {
                        sp128[k].r = (k < 9) ? 30 : 30 - (k - 8) * 3;
                        sp128[k].g = (k < 9) ? 30 : 30 - (k - 8) * 3;
                        sp128[k].b = (k < 9) ? 20 : 20 - (k - 8) * 2;
                    }
                } else if (s3[0x28 / 4] == 1) {
                    sp128[k].r = (k < 9) ? 70 : 70 - (k - 8) * 7;
                    sp128[k].g = (k < 9) ? 20 : 20 - (k - 8) * 2;
                    sp128[k].b = 0;
                } else if (s3[0x28 / 4] == 2) {
                    sp128[k].r = 0;
                    sp128[k].g = (k < 9) ? 20 : 20 - (k - 8) * 2;
                    sp128[k].b = (k < 9) ? 30 : 30 - (k - 8) * 3;
                }
                s3[0x2C / 4] = (k == 0) ? 5 : 10;
                for (j = 0; j < 2; j++) {
                    s3[0x30 / 4] = k * 2 + j;
                    s3[0x34 / 4] = s3[0x28 / 4] * 16 + s3[0x28 / 4] * 4 + k + 1;
                    sp638[s3[0x30 / 4]].v0.vx = ObjTrailHistory[s3[0x34 / 4]].vx;
                    sp638[s3[0x30 / 4]].v0.vy = ObjTrailHistory[s3[0x34 / 4]].vy - s3[0x2C / 4] * j;
                    sp638[s3[0x30 / 4]].v0.vz = ObjTrailHistory[s3[0x34 / 4]].vz - s3[0x2C / 4] * (1 - j);
                    sp638[s3[0x30 / 4]].v1.vx = ObjTrailHistory[s3[0x34 / 4]].vx;
                    sp638[s3[0x30 / 4]].v1.vy = ObjTrailHistory[s3[0x34 / 4]].vy + s3[0x2C / 4] * j;
                    sp638[s3[0x30 / 4]].v1.vz = ObjTrailHistory[s3[0x34 / 4]].vz + s3[0x2C / 4] * (1 - j);
                    sp638[s3[0x30 / 4]].v2.vx = ObjTrailHistory[s3[0x34 / 4] + 1].vx;
                    sp638[s3[0x30 / 4]].v2.vy = ObjTrailHistory[s3[0x34 / 4] + 1].vy - j * 10;
                    sp638[s3[0x30 / 4]].v2.vz = ObjTrailHistory[s3[0x34 / 4] + 1].vz - (1 - j) * 10;
                    sp638[s3[0x30 / 4]].v3.vx = ObjTrailHistory[s3[0x34 / 4] + 1].vx;
                    sp638[s3[0x30 / 4]].v3.vy = ObjTrailHistory[s3[0x34 / 4] + 1].vy + j * 10;
                    sp638[s3[0x30 / 4]].v3.vz = ObjTrailHistory[s3[0x34 / 4] + 1].vz + (1 - j) * 10;
                    sp638[s3[0x30 / 4]].c0 = sp638[s3[0x30 / 4]].c1 = (k == 0) ? sp128[0] : sp128[k - 1];
                    sp638[s3[0x30 / 4]].c2 = sp638[s3[0x30 / 4]].c3 = sp128[k];
                }
            }
            BuildObjectMatrix(i, &sp98, &sp18, arg0);
            sp120.vx = -SCRATCH_CAMX >> 3;
            sp120.vy = -SCRATCH_CAMY >> 3;
            sp120.vz = -SCRATCH_CAMZ >> 3;
            ApplyMatrix(SCRATCH_MAT, &sp120, &sp1C8);
            sp38.t[0] = sp1C8.vx;
            sp38.t[1] = sp1C8.vy;
            sp38.t[2] = sp1C8.vz;
            RR_GTE_SETROT(&sp18);
            RR_GTE_SETTR_AT20(&sp38);
            if ((TrackObjectDefs[i].f0C % 10) == 5) {
                EmitSemiTransG4List(sp638, 36, 2);
            } else {
                EmitSemiTransG4List(sp638, 36, 0);
            }
            break;

        case 3:
            for (k = 0; k < 38; k++) {
                sp128[k].r = (k < 21) ? CourseColorTable[CourseIndex - 4].r : CourseColorTable[CourseIndex - 4].r - (CourseColorTable[CourseIndex - 4].r * (k - 20)) / 17;
                sp128[k].g = (k < 21) ? CourseColorTable[CourseIndex - 4].g : CourseColorTable[CourseIndex - 4].g - (CourseColorTable[CourseIndex - 4].g * (k - 20)) / 17;
                sp128[k].b = (k < 21) ? CourseColorTable[CourseIndex - 4].b : CourseColorTable[CourseIndex - 4].b - (CourseColorTable[CourseIndex - 4].b * (k - 20)) / 17;
                for (j = 0; j < 2; j++) {
                    s3[0x38 / 4] = k * 2 + j;
                    sp638[s3[0x38 / 4]].v0.vx = ObjTrailHistory[k + 1].vx;
                    sp638[s3[0x38 / 4]].v0.vy = ObjTrailHistory[k + 1].vy;
                    sp638[s3[0x38 / 4]].v0.vz = ObjTrailHistory[k + 1].vz;
                    sp638[s3[0x38 / 4]].v1.vx = ObjTrailHistory[k + 1].vx;
                    sp638[s3[0x38 / 4]].v1.vy = ObjTrailHistory[k + 1].vy - (k * 2 + 25) + (k * 4 + 50) * (1 - j);
                    sp638[s3[0x38 / 4]].v1.vz = ObjTrailHistory[k + 1].vz;
                    sp638[s3[0x38 / 4]].v2.vx = ObjTrailHistory[k + 2].vx;
                    sp638[s3[0x38 / 4]].v2.vy = ObjTrailHistory[k + 2].vy;
                    sp638[s3[0x38 / 4]].v2.vz = ObjTrailHistory[k + 2].vz;
                    sp638[s3[0x38 / 4]].v3.vx = ObjTrailHistory[k + 2].vx;
                    sp638[s3[0x38 / 4]].v3.vy = ObjTrailHistory[k + 2].vy - (k * 2 + 25) + (k * 4 + 50) * (1 - j);
                    sp638[s3[0x38 / 4]].v3.vz = ObjTrailHistory[k + 2].vz;
                    sp638[s3[0x38 / 4]].c0 = sp638[s3[0x38 / 4]].c2 = (k == 0) ? sp128[0] : sp128[k - 1];
                    sp638[s3[0x38 / 4]].c1 = sp638[s3[0x38 / 4]].c3 = sp128[37];
                }
            }
            for (j = 0; j < 2; j++) {
                sp638[j + 76].v0.vx = TrackObjectStates[i].f0C >> 3;
                sp638[j + 76].v0.vy = TrackObjectStates[i].f10 >> 3;
                sp638[j + 76].v0.vz = TrackObjectStates[i].f14 >> 3;
                sp638[j + 76].v1.vx = TrackObjectStates[i].f0C >> 3;
                sp638[j + 76].v1.vy = TrackObjectStates[i].f10 >> 3;
                sp638[j + 76].v1.vz = TrackObjectStates[i].f14 >> 3;
                sp638[j + 76].v2.vx = ObjTrailHistory[1].vx;
                sp638[j + 76].v2.vy = ObjTrailHistory[1].vy;
                sp638[j + 76].v2.vz = ObjTrailHistory[1].vz;
                sp638[j + 76].v3.vx = ObjTrailHistory[1].vx;
                sp638[j + 76].v3.vy = ObjTrailHistory[1].vy - 25 + 50 * (1 - j);
                sp638[j + 76].v3.vz = ObjTrailHistory[1].vz;
                sp638[j + 76].c0 = sp638[j + 76].c2 = sp128[1];
                sp638[j + 76].c1 = sp638[j + 76].c3 = sp128[37];
            }
            BuildObjectMatrix(i, &sp98, &sp18, arg0);
            sp120.vx = -SCRATCH_CAMX >> 3;
            sp120.vy = -SCRATCH_CAMY >> 3;
            sp120.vz = -SCRATCH_CAMZ >> 3;
            ApplyMatrix(SCRATCH_MAT, &sp120, &sp1C8);
            sp38.t[0] = sp1C8.vx;
            sp38.t[1] = sp1C8.vy;
            sp38.t[2] = sp1C8.vz;
            RR_GTE_SETROT(&sp18);
            RR_GTE_SETTR_AT20(&sp38);
            EmitSemiTransG4List(sp638, 78, 0);
            break;

        case 4:
            {
                s32 dx = (TrackObjectStates[i].f0C - SCRATCH_CAMX) >> 4;
                s32 dz = (TrackObjectStates[i].f14 - SCRATCH_CAMZ) >> 4;
                s3[0x00 / 4] = CordicSqrt((dx * dx + dz * dz) >> 12);
            }
            if ((u32)((u16)RaceType - 2) < 3U) {
                if (D_8010E848[i] != ModeFrame && RacePaused == 0) {
                    TrackObjectStates[i].f08 = (TrackObjectStates[i].f08 + (((i & 1) == 0) ? 0x20 : 0xFE0)) % 0x1000;
                }
                D_8010E848[i] = ModeFrame;
            } else if (RacePaused == 0) {
                TrackObjectStates[i].f08 = (TrackObjectStates[i].f08 + (((i & 1) == 0) ? 0x20 : 0xFE0)) % 0x1000;
            }
            if (s3[0x00 / 4] < 900) {
                for (j = 0; j < 2; j++) {
                    spA0[j].vx = 0;
                    spA0[j].vy = -1000 - s3[0x00 / 4];
                    { s32 t = ((((s3[0x00 / 4] - 500) * 3 >> 2) + 900) * rsin(TrackObjectStates[i].f08) >> 12) - 400; spA0[j].vz = j * (((s3[0x00 / 4] - 500) / 7) + 400) + t; }
                    sp128[j].b = 0;
                    sp128[j].g = 0;
                    sp128[j].r = 0;
                }
                {
                    k = (PsyqRand() % 10) + (TrackObjectStates[i].f04 % 100);
                if (RacePaused != 0) {
                    k = TrackObjectStates[i].f04 % 100;
                }
                    for (j = 0; j < 3; j++) {
                        sp1D8[j].v0.vy = 20;
                        sp1D8[j].v0.vz = 50;
                        sp1D8[j].v0.vx = 0;
                        sp1D8[j].c0.r = k >> 1;
                        sp1D8[j].c0.g = sp1D8[j].c0.b = (k * 2) / 3;
                        sp1D8[j].v1 = spA0[0];
                        sp1D8[j].v2 = spA0[1];
                        sp1D8[j].c1 = sp128[0];
                        sp1D8[j].c2 = sp128[1];
                        switch (j) {
                        case 0:
                            sp1D8[0].v1 = spA0[0];
                            sp1D8[0].v2 = spA0[1];
                            break;
                        case 1:
                        case 2:
                            s3[0x20 / 4] = 200 - (s3[0x00 / 4] - 500) / 14;
                            s3[0x24 / 4] = (s3[0x20 / 4] * 17) / 10;
                            sp1D8[j].v2.vx = -(u16)s3[0x24 / 4];
                            sp1D8[j].v2.vy = spA0[0].vy;
                            sp1D8[j].v2.vz = spA0[0].vz + (u16)s3[0x20 / 4];
                            sp1D8[j].v1 = spA0[j - 1];
                            break;
                        }
                    }
                }
                BuildObjectMatrix(i, &TrackObjectStates[i].rot, &sp18, arg0);
                RR_GTE_SETROT(&sp18);
                EmitSemiTransG3List(sp1D8, 3, 0);
            } else if (s3[0x00 / 4] < 3300) {
                for (j = 0; j < 2; j++) {
                    spA0[j].vx = 0;
                    spA0[j].vy = -1000 - s3[0x00 / 4];
                    { s32 t = ((((s3[0x00 / 4] - 500) * 3 >> 2) + 900) * rsin(TrackObjectStates[i].f08) >> 12) - 400; spA0[j].vz = j * (((s3[0x00 / 4] - 500) / 7) + 400) + t; }
                    sp128[j].b = 0;
                    sp128[j].g = 0;
                    sp128[j].r = 0;
                }
                {
                    k = (PsyqRand() % 10) + (TrackObjectStates[i].f04 % 100);
                if (RacePaused != 0) {
                    k = TrackObjectStates[i].f04 % 100;
                }
                    sp1D8[0].v0.vy = 20;
                    sp1D8[0].v0.vx = 0;
                    sp1D8[0].v0.vz = 50;
                    sp1D8[0].c0.r = k;
                    sp1D8[0].c0.g = sp1D8[0].c0.b = (k * 4) / 3;
                    sp1D8[0].v1 = spA0[0];
                    sp1D8[0].v2 = spA0[1];
                    sp1D8[0].c1 = sp128[0];
                    sp1D8[0].c2 = sp128[1];
                    sp1D8[0].v1 = spA0[0];
                    sp1D8[0].v2 = spA0[1];
                }
                BuildObjectMatrix(i, &TrackObjectStates[i].rot, &sp18, arg0);
                RR_GTE_SETROT(&sp18);
                EmitSemiTransG3List(sp1D8, 1, 0);
            } else {
                for (j = 0; j < 2; j++) {
                    spA0[j].vx = 0;
                    spA0[j].vy = -350 - (s3[0x00 / 4] >> 1);
                    { s32 t = ((rsin(TrackObjectStates[i].f08) * 3000) >> 12) - 400; spA0[j].vz = j * 800 + t; }
                    sp128[j].b = 0;
                    sp128[j].g = 0;
                    sp128[j].r = 0;
                }
                {
                    k = (PsyqRand() % 10) + (TrackObjectStates[i].f04 % 100);
                if (RacePaused != 0) {
                    k = TrackObjectStates[i].f04 % 100;
                }
                    if (s3[0x00 / 4] >= 5801) {
                        k = (k * 500) / (s3[0x00 / 4] - 5300);
                    }
                    {
                        s32 x, y, z;
                        x = TrackObjectStates[i].f0C >> 3;
                        sp1D8[0].v0.vx = x;
                        y = TrackObjectStates[i].f10 >> 3;
                        sp1D8[0].v0.vy = y;
                        z = TrackObjectStates[i].f14 >> 3;
                        sp1D8[0].v1.vx = spA0[0].vx + x;
                        sp1D8[0].v1.vy = spA0[0].vy + y;
                        sp1D8[0].c0.r = k;
                        sp1D8[0].v2.vx = spA0[1].vx + x;
                        sp1D8[0].v2.vy = spA0[1].vy + y;
                        sp1D8[0].v0.vz = z;
                        sp1D8[0].v1.vz = z + (spA0[0].vz >> 1);
                        sp1D8[0].v2.vz = z + (spA0[1].vz >> 1);
                    }
                    sp1D8[0].c0.g = sp1D8[0].c0.b = (k * 4) / 3;
                    sp1D8[0].c1 = sp128[0];
                    sp1D8[0].c2 = sp128[1];
                }
                BuildObjectMatrix(i, &sp98, &sp18, arg0);
                sp120.vx = -SCRATCH_CAMX >> 3;
                sp120.vy = -SCRATCH_CAMY >> 3;
                sp120.vz = -SCRATCH_CAMZ >> 3;
                ApplyMatrix(SCRATCH_MAT, &sp120, &sp1C8);
                sp38.t[0] = sp1C8.vx;
                sp38.t[1] = sp1C8.vy;
                sp38.t[2] = sp1C8.vz;
                RR_GTE_SETROT(&sp18);
                RR_GTE_SETTR_AT20(&sp38);
                EmitSemiTransG3List(sp1D8, 1, 2);
            }
            break;
        }
    }
}

/* legacy typing of shared globals in this part of the unit */
#define MulMatrix0 ((int (*)())MulMatrix0)

void BuildObjectMatrix(int unused, short *p, M *out, int flag) {
    M m10, m30, m50, m70;
    M m90;
    M *q = &m70;
    BuildRotMatrixZ(q, -p[2]);
    BuildRotMatrixX(&m30, -p[0]);
    if (flag == 2) {
        m90 = *(M *)SCRATCH_ROT_MATRIX;
        BuildRotMatrixY(&m50, -p[1] - 0x800);
        ((unsigned short *)&m90)[6] = -((unsigned short *)&m90)[6];
        ((unsigned short *)&m90)[7] = -((unsigned short *)&m90)[7];
        ((unsigned short *)&m90)[8] = -((unsigned short *)&m90)[8];
        MulMatrix2(&m90, q);
    } else {
        BuildRotMatrixY(&m50, -p[1]);
        MulMatrix2((int *)SCRATCH_ROT_MATRIX, q);
    }
    MulMatrix0(&m30, &m50, &m10);
    MulMatrix2(&m70, &m10);
    *out = m10;
}

#undef MulMatrix0

/* legacy typing of shared globals in this part of the unit */
#define TrackObjectStates ((char *)TrackObjectStates)

void SetTrackObjectTransform(int a0, int *a1, int a2)
{
  short sp10[4];
  int sp18[4];
  int *sp = (int *) SCRATCHPAD;
  char *p;
  int v;
  RR_GTE_SETROT(CameraViewMatrix);
  sp[0x78 / 4] = 0;
  sp10[0] = ((*((int *) ((((char *) (a0 * 0x2C)) + ((int) TrackObjectStates)) + 0xC))) - sp[2]) >> 2;
  sp10[1] = ((*((int *) ((((char *) (a0 * 0x2C)) + ((int) TrackObjectStates)) + 0x10))) - sp[3]) >> 2;
  sp10[2] = ((*((int *) ((((char *) (a0 * 0x2C)) + ((int) TrackObjectStates)) + 0x14))) - sp[4]) >> 2;
  ApplyRotMatrix(sp10, sp18);
  if (a2 == 2)
  {
    v = -sp18[0];
  }
  else
  {
    v = sp18[0];
  }
  a1[5] = (v << 1) << 1;
  a1[6] = sp18[1] << 2;
  a1[7] = sp18[2] << 2;
  RR_GTE_SETTR_AT20(a1);
  RR_GTE_SETROT(a1);
}

#undef TrackObjectStates

/* legacy typing of shared globals in this part of the unit */
#define TrackObjectKeyframes ((int *)TrackObjectKeyframes)
#define TrackObjectDefs ((int *)TrackObjectDefs)
#define TrackObjectStates (*(int (*))&TrackObjectStates)

int LoadTrackObjects(int *a, int b) {
    int n = *a;
    int s;
    a++;
    D_8010E8DC = a;
    TrackObjectDefs = a;
    D_8010E8E0 = b;
    TrackObjectStates = b;
    a = a + n * 12;
    TrackObjectCount = n;
    D_8010E8D8 = a;
    s = n * 0x2C;
    TrackObjectKeyframes = a;
    PsyqBzero(b, s);
    return s;
}

#undef TrackObjectKeyframes
#undef TrackObjectDefs
#undef TrackObjectStates
