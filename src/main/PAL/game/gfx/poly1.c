#include "types.h"
#include "scratchpad.h"
#include "m2c_macros.h"
#include "game/camera/viewcam.h"
#include "game/gfx/model.h"
#include "psyq/libgpu/addprim3.h"
#include "psyq/libgpu/addprim4.h"
#include "psyq/libgpu/primatt5.h"
#include "psyq/libgpu/primatt7.h"
#include "psyq/libgte/gtecolo3.h"
#include "psyq/libgte/gtemat3.h"
#include "psyq/libgte/gtemat7.h"
#include "game/gfx/poly1.h"
#include "rr/race_setup.h"
#include "rr/matrix_workspace.h"
#include "rr/audio_loop.h"
#include "rr/memory.h"
#include "rr/overlay661_init.h"
#include "rr/hud_number.h"
#include "rr/relative_file.h"
#include "rr/frame_environment.h"
#include "game/demo/splash.h"
#include "rr/animated_glyph.h"
#include "game/data/globals.h"

typedef struct { int a[8]; } S32B;

typedef struct { unsigned short x, y, z; } V3;

struct P { int w; int x; short a, b, c, d; };

typedef struct {
    int f0, f4, f8, fC, f10, f14, f18, f1C, f20, f24, f28, f2C, f30, f34, f38, f3C, f40, f44;
    int t48[16];
    int t88[16];
    int fC8, fCC, fD0, fD4, fD8, fDC, fE0, fE4, fE8, fEC, fF0, fF4;
} Q;

typedef struct { int a[8]; } M;

typedef struct { unsigned char c[4]; } U4;

typedef struct { int x, y; } __attribute__((packed)) U8;

typedef struct { short h0, h2, h4, h6, h8, hA, hC, hE; unsigned char c10[8]; unsigned char c18[8]; int f20; } P;

typedef struct { U4 u; int n; } K;

typedef struct {
    P *p0; int f4, f8, fC, f10, f14, f18, f1C, p20, p24, f28, f2C, f30, f34, f38, p3C;
    K *k40; K *k44;
    U4 c48[16]; U4 c88[16];
    int fC8, fCC, fD0, pD4[5]; int e8[2]; int f0[2];
} O;

struct S { unsigned char c[8]; };

extern int *CountedOffsetTablePtr;
extern s32 RearViewMirrorReady;
extern int BuildRotMatrixY();
extern int BuildRotMatrixX();
extern int MulMatrix0();
extern short RearViewPitch;
extern S32B RearViewRotMatrix;
extern V3 CameraViewMatrix;
extern int RaceState;
extern V3 CameraPitchRollMatrix;
extern S32B RearViewSavedRotWord;
extern int DrawCars();
extern int DrawTrackSceneryForPlayer();
extern int SetupViewport();
extern int DrawTrackObjects();
extern int LineTrailDataBase;
extern int TrackParticleKindTableA[];
extern int TrackParticleKindTableB[];
extern int TrackObjectParticleStates;
extern int VSync();
extern int PsyqSrand();
extern short D_80107180[];
extern int BuildRotMatrixZ();
extern int MulMatrix();
extern M HalfTransViewMatrix;
extern int ApplyMatrixSV();
extern int StepDrawParticleLines();
extern int SquareRoot0();
extern int Atan2Table();
extern short D_80107182;
extern short D_80107184;
extern struct { short m[3][3]; int t[3]; } ColorMatrix;
extern struct { short a, b, c, d; } LightAngles[];
extern short *SegmentOctantTable;
extern unsigned char *OctantLookupTable;
extern int SetDispMask();
extern int VSyncThreshold;
extern struct S SplashLogoClutRect;
extern struct S SplashClutRect;
extern struct S SplashLogoImageRect;
extern struct S SplashLogoRect4;
extern struct S SplashLogoParam5;

void RelocateCountedOffsetTable(int *a0);
int SetupRearViewMirror(void);
void SetupRearViewMatrix(void);
struct P *AddRearViewMirrorTile(struct P *a0);
void DrawRearViewMirror(int arg0);
void ConfigureTrackObjectParticles(int a0, int a1, int *a2, int a3);
void SetupObjectRotTransMatrix(char *p);
void UpdateTrackObjectParticles(char *a0, int a1, int a2);
s32 GetRelativeFileTableSize(void);
s32 GetRelativeFileSlotCount(void);

void RelocateCountedOffsetTable(int *a0) {
    int *p = a0 + 1;
    int i = 0;
    int n = *a0 * 8;
    CountedOffsetTablePtr = p;
    if (n > 0) {
        do {
            i++;
            *p += (int)a0;
            p++;
        } while (i < n);
    }
}

void ResetRearViewMirror(void) {
    RearViewMirrorEnabled = 1;
    RearViewMirrorOffsetY = -0x24;
    RearViewMirrorReady = 0;
}

int SetupRearViewMirror(void) {
    char sp[32];
    int r = 0;
    int *s = (int *)SCRATCHPAD;
    if (RearViewMirrorReady != 0) {
        if (RearViewMirrorEnabled != 0) {
            if (RearViewMirrorHidden == 0) {
                if (RaceType != 0) {
                    r = RaceState == 2;
                }
            }
        }
    }
    if (r != 0) {
        RearViewSavedRotWord = *(S32B *)(s + 10);
        *(S32B *)(s + 10) = RearViewRotMatrix;
        s[7] += 0x800;
        BuildRotMatrixY(sp, 0x800);
        MulMatrix0(sp, &CameraViewMatrix, &CameraViewMatrix);
        MulMatrix0(sp, &CameraPitchRollMatrix, &CameraPitchRollMatrix);
        BuildRotMatrixX(sp, RearViewPitch * 2);
        MulMatrix2(sp, &CameraViewMatrix);
        MulMatrix2(sp, &CameraPitchRollMatrix);
        CameraViewMatrix.x = -CameraViewMatrix.x;
        CameraViewMatrix.y = -CameraViewMatrix.y;
        CameraViewMatrix.z = -CameraViewMatrix.z;
        CameraPitchRollMatrix.x = -CameraPitchRollMatrix.x;
        CameraPitchRollMatrix.y = -CameraPitchRollMatrix.y;
        CameraPitchRollMatrix.z = -CameraPitchRollMatrix.z;
    }
    return r;
}

void SetupRearViewMatrix(void) {
    int *sp = (int *)SCRATCHPAD;
    sp[7] -= 0x800;
    *(S32B *)SCRATCH_ROT_MATRIX = RearViewSavedRotWord;
}

/* legacy typing of shared globals in this part of the unit */
#define RearViewMirrorOffsetY (*(unsigned short (*))&RearViewMirrorOffsetY)

struct P *AddRearViewMirrorTile(struct P *a0) {
    
    int s1 = *(int *)(SCRATCHPAD + 0x4);
    SetTile(a0);
    a0->a = 0x60;
    a0->c = 0x80;
    ((char *)a0)[4] = 0;
    ((char *)a0)[5] = 0;
    ((char *)a0)[6] = 0;
    a0->d = 0x20;
    a0->b = RearViewMirrorOffsetY - 2;
    AddPrim(s1, a0++);
    return a0;
}

#undef RearViewMirrorOffsetY

void DrawRearViewMirror(int arg0) {
    if (arg0 >= 0x169) {
        RearViewMirrorReady = 1;
    }
    if (RearViewMirrorReady != 0) {
        if (RearViewMirrorEnabled != 0) {
            if (RearViewMirrorOffsetY < 0x12) {
                RearViewMirrorOffsetY += 1;
            }
        } else if (RearViewMirrorOffsetY >= -0x23) {
            RearViewMirrorOffsetY -= 1;
        }
        if (SetupRearViewMirror() != 0) {
            *(int *)SCRATCHPAD = AddRearViewMirrorTile(*(int *)SCRATCHPAD);
            SetupViewport(1);
            DrawTrackSceneryForPlayer(0);
            PrepareRearViewFrame(0);
            DrawTrackObjects(2);
            DrawCars(1);
            SetupRearViewMatrix();
        }
    }
}

void InitLineG2TrailPool(a0)
    int a0;
{
    int s0, s1, s2, s4;
    register int s3 asm("$19");
    int *p, *q, *r;
    int b;
    int t3;
    char *m;
    LineTrailDataBase = a0;
    a0 = a0 + 0xB34C;
    s0 = a0 + 0x4D8;
    s3 = 0;
    TrackObjectParticleStates = a0;
    do {
        s4 = 0;
        s2 = 0;
        do {
            t3 = s3 * 4;
            m = (char *)TrackObjectParticleStates + s2;
            m = m + t3;
            *(int *)(m + 0xE8) = s0;
            *(int *)(m + 0xF0) = 0;
            SetLineG2(s0);
            s0 += 0x14;
            s1 = 1;
            do {
                SetLineG2(s0);
                CatPrim(s0 - 0x14, s0);
                s1++;
                s0 += 0x14;
            } while (s1 < 0xFF);
            s4++;
            s2 += 0xF8;
        } while (s4 < 5);
        s3++;
    } while (s3 < 2);
    p = (int *)s0;
    b = s0;
    s3 = 0;
    q = TrackParticleKindTableB;
    r = TrackParticleKindTableA;
    do {
        *r = b + *p;
        p++;
        s3++;
        *q = b + *p;
        p++;
        r++;
        q++;
    } while (s3 < 9);
}

/* legacy typing of shared globals in this part of the unit */
#define TrackObjectParticleStates ((char *)TrackObjectParticleStates)

void ConfigureTrackObjectParticles(int a0, int a1, int *a2, int a3) {
    Q *t = (Q *)(TrackObjectParticleStates + a0 * 0xF8);
    int f;
    int i;
    int *p;
    if (a1 & 1) {
        if (a2 != 0) {
            t->fC8 = a2[0];
            t->fCC = a2[1];
            t->fD0 = a2[2];
        }
    }
    if (a1 & 2) {
        if (a3 >= 0) t->f3C = a3;
    }
    f = a1 & 4;
    a1 = 0;
    if (f) {
        t->f8 = 0xFF;
        t->fC = 5;
        t->f20 = 0x1000;
        t->f24 = 0x1000;
        t->f28 = 0x20;
        t->f2C = 0xB5;
        t->f30 = 1;
        t->f4 = 0;
        t->f10 = 0;
        t->f14 = 0;
        t->f18 = 0;
        t->f1C = 0;
        t->f34 = 0;
        t->f38 = 0;
        t->f0 = LineTrailDataBase + a0 * 0x23DC;
        t->f40 = TrackParticleKindTableA[t->f3C];
        t->f44 = TrackParticleKindTableB[t->f3C];
        do {
            t->t48[a1] = 0x52000000;
            t->t88[a1] = 0x52000000;
            a1++;
        } while (a1 < 0x10);
        t->fD8 = 0;
        t->fDC = 0;
        t->fE0 = 0;
        t->fF0 = 0;
        t->fF4 = 0;
        D_80107180[0] = 0x420;
        D_80107180[1] = 0x5D0;
        D_80107180[2] = 0x610;
        PsyqSrand(VSync(1, a1));
    }
}

#undef TrackObjectParticleStates

void SetupObjectRotTransMatrix(char *p) {
    M m10, m30;
    int v[3];
    BuildRotMatrixY(&m10, *(int *)(p + 0xDC));
    BuildRotMatrixX(&m30, *(int *)(p + 0xD8));
    MulMatrix2(&m30, &m10);
    BuildRotMatrixZ(&m30, *(int *)(p + 0xE0));
    MulMatrix2(&m30, &m10);
    ApplyMatrixLV(&HalfTransViewMatrix, p + 0xC8, v);
    m10.a[5] = (HalfTransViewMatrix.a[5] + v[0]) >> 1;
    m10.a[6] = (HalfTransViewMatrix.a[6] + v[1]) >> 1;
    m10.a[7] = (HalfTransViewMatrix.a[7] + v[2]) >> 1;
    SetTransMatrix(&m10);
    MulMatrix(&m10, &HalfTransViewMatrix);
    SetRotMatrix(&m10);
}

/* legacy typing of shared globals in this part of the unit */
#define TrackObjectParticleStates ((O *)TrackObjectParticleStates)

void UpdateTrackObjectParticles(char *a0, int a1, int a2) {
    int sp10[3];
    int sp20[8];
    int sp40[8];
    short sp60[4];
    int sp68, sp6c, sp70;
    O *s1; /*D*/
    P *s0; /*D*/
    K *k; /*D*/
    int s2; /*D*/
    int s3; /*D*/
    int s7; /*D*/
    int *g = (int *)(SCRATCHPAD + 0x8); /*D*/
    int a; /*D*/
    int b; /*D*/
    int c; /*D*/
    int d; /*D*/
    unsigned char *q; /*D*/

    PsyqMemset(sp60, 0, 8);
    sp60[2] = 0x100;
    s1 = TrackObjectParticleStates + a1;
    s7 = *(int *)(a0 + 4) + 0x900;
    s1->f30 = a2;
    s2 = (s1->f14 - s1->f28) & 0xF;
    if (s1->f10 < s1->f8) {
        a1 = 0;
        while (a1 < 10) {
            if (!(s1->f10 < s1->f8)) break;
            s0 = s1->p0 + s1->f10;
            s0->h0 = 0;
            s0->h2 = 0;
            s0->h4 = 0;
            s0->h8 = 0;
            s0->hA = 0;
            s0->hC = 0;
            s0->f20 = PsyqRand() & 0xF;
            BuildRotMatrixY(sp20, PsyqRand() & 0xFFF);
            a1++;
            BuildRotMatrixX(sp40, PsyqRand() & 0xFFF);
            MulMatrix2(sp40, sp20);
            BuildRotMatrixZ(sp40, PsyqRand() & 0xFFF);
            MulMatrix2(sp40, sp20);
            ApplyMatrixSV(sp20, sp60, s0->c10);
            *(U8 *)s0->c18 = *(U8 *)s0->c10;
            s1->f10 = s1->f10 + 1;
        }
    } else {
        a = s1->f0[DrawBufIndex];
        s3 = s1->e8[DrawBufIndex];
        if (a != 0) CatPrim(a, a + 0x14);
        __asm__ volatile ("cfc2 $8, $21\n\tcfc2 $9, $22\n\tcfc2 $10, $23\n\tsw $8, 0(%0)\n\tsw $9, 0(%1)\n\tsw $10, 0(%2)"
            : : "r"(&sp68), "r"(&sp6c), "r"(&sp70) : "$8", "$9", "$10", "memory");
        if (s1->f14 > s1->f2C) {
            ConfigureTrackObjectParticles(a1, 4, 0, -1);
        } else {
            a = s1->f28;
            if (s1->f14 < a) {
                if (s1->f30) s1->f14 = s1->f14 + 1;
                s1->f4 = 0;
                s1->f0[DrawBufIndex] = 0;
            } else {
                b = s1->f14 - a + 1;
                s1->f4 = b;
                if (b >= 0x10) s1->f4 = s1->f8;
                else s1->f4 = b * (s1->f8 / 16);
                if (s1->f30) s1->f14 = s1->f14 + 1;
                k = s1->k40 + s1->f34;
                if (k->n == -1) {
                    s1->c48[s2] = k->u;
                } else {
                    __asm__ volatile ("ctc2 %0, $21\n\tctc2 %1, $22\n\tctc2 %2, $23" : : "r"(k[1].u.c[0] << 4), "r"(k[1].u.c[1] << 4), "r"(k[1].u.c[2] << 4));
                    d = (s1->f18 << 12) / k->n;
                    DpqColor(k, d, &s1->c48[s2]);
                    if (s1->f30) {
                        s1->f18 = s1->f18 + 1;
                        if (s1->f18 >= k->n) {
                            s1->f18 = 0;
                            s1->f34 = s1->f34 + 1;
                        }
                    }
                }
                k = s1->k44 + s1->f38;
                if (k->n == -1) {
                    s1->c88[s2] = k->u;
                } else {
                    __asm__ volatile ("ctc2 %0, $21\n\tctc2 %1, $22\n\tctc2 %2, $23" : : "r"(k[1].u.c[0] << 4), "r"(k[1].u.c[1] << 4), "r"(k[1].u.c[2] << 4));
                    d = (s1->f1C << 12) / k->n;
                    DpqColor(k, d, &s1->c88[s2]);
                    if (s1->f30) {
                        s1->f1C = s1->f1C + 1;
                        if (s1->f1C >= k->n) {
                            s1->f1C = 0;
                            s1->f38 = s1->f38 + 1;
                        }
                    }
                }
                SetupObjectRotTransMatrix(s1);
                s3 = StepDrawParticleLines(a0, s3, s1, s2 << 2);
                if (s3 != s1->e8[DrawBufIndex]) s1->f0[DrawBufIndex] = s3 - 0x14;
                else s1->f0[DrawBufIndex] = 0;
                if (a1 < 2) {
                    if (s1->c48[s2].c[0] * 16 > D_80107180[0]) ColorMatrix.m[0][a1] = (short)s1->c48[s2].c[0] << 4;
                    else ColorMatrix.m[0][a1] = D_80107180[0];
                    if (s1->c48[s2].c[1] * 16 > D_80107180[1]) ColorMatrix.m[1][a1] = (short)s1->c48[s2].c[1] << 4;
                    else ColorMatrix.m[1][a1] = D_80107180[1];
                    if (s1->c48[s2].c[2] * 16 > D_80107180[2]) ColorMatrix.m[2][a1] = (short)s1->c48[s2].c[2] << 4;
                    else ColorMatrix.m[2][a1] = D_80107180[2];
                }
            }
        }
        c = s1->f0[DrawBufIndex];
        if (c != 0) {
            AddPrims(s7, s1->e8[DrawBufIndex], c);
        } else if (a1 < 2) {
            ColorMatrix.m[0][a1] = D_80107180[0];
            ColorMatrix.m[1][a1] = D_80107180[1];
            ColorMatrix.m[2][a1] = D_80107180[2];
        }
        if (a1 < 2) {
            sp10[0] = (g[0] >> 2) - 0x8000;
            sp10[1] = g[1] >> 2;
            sp10[2] = (g[2] >> 2) - 0x8000;
            sp10[0] = (sp10[0] >> 1) - s1->fC8;
            sp10[1] = (sp10[1] >> 1) - s1->fCC;
            sp10[2] = (sp10[2] >> 1) - s1->fD0;
            LightAngles[a1].a = -Atan2Table(SquareRoot0((sp10[0] >> 4) * (sp10[0] >> 4) + (sp10[2] >> 4) * (sp10[2] >> 4)), sp10[1] >> 4) & 0xFFF;
            LightAngles[a1].b = (0x400 - Atan2Table(sp10[0], sp10[2])) & 0xFFF;
            LightAngles[a1].c = 0;
        }
        __asm__ volatile ("ctc2 %0, $21\n\tctc2 %1, $22\n\tctc2 %2, $23" : : "r"(sp68), "r"(sp6c), "r"(sp70));
    }
}

#undef TrackObjectParticleStates

s32 GetRelativeFileTableSize(void) {
    return RelativeFileCount * 0x10;
}

void ApplySegmentOctantValue(void) {
    int a, b;
    short *p;
    short v;
    int base;
    register short *g __asm__("$17");
    a = GetViewTrackSegment((SCRATCHPAD + 0x8), 0);
    b = YawToOctant((SCRATCHPAD + 0x8));
    base = (int)SegmentOctantTable;
    p = (short *)(b * 2 + ((a << 4) + base));
    if (*p >= 0) {
        v = *(unsigned short *)p;
    } else {
        v = DefaultSegmentOctantValue;
    }
    /* ASM_BARRIER: legacy, reason not recorded (forces reg copy/CSE/scheduling); retire & retest with fd.py */
    __asm__("" : "=r"(g) : "0"(SCRATCHPAD));
    g[0x2E] = v;
}

/* legacy typing of shared globals in this part of the unit */
#define SegmentOctantTable (*(s32 (*))&SegmentOctantTable)

void SetSegmentOctantTable(s32 arg0) {
    SegmentOctantTable = arg0;
}

#undef SegmentOctantTable

s32 GetRelativeFileSlotCount(void) {
    return RelativeFileCount * 8;
}

int GetOctantTableValue(void) {
    int a = GetViewTrackSegment((SCRATCHPAD + 0x8), 0);
    return (OctantLookupTable + a * 8)[YawToOctant((SCRATCHPAD + 0x8))];
}

/* legacy typing of shared globals in this part of the unit */
#define OctantLookupTable (*(s32 (*))&OctantLookupTable)

void SetOctantLookupTable(s32 arg0) {
    OctantLookupTable = arg0;
}

#undef OctantLookupTable

void SplashLogoInit(void) {
    struct S a, b, c, d, e, f;
    a = SplashLogoClutRect;
    b = SplashLogoClutRect;
    c = SplashClutRect;
    d = SplashLogoImageRect;
    e = SplashLogoRect4;
    f = SplashLogoParam5;
    SetDispMask(0);
    SetupDrawDispEnvs(0, 0, 0, 3);
    VSyncThreshold = 0x80;
    SplashLogoFrame = 0;
}
