#include "game/car/car.h"
#include "types.h"
#include "scratchpad.h"
#include "m2c_macros.h"
#include "game/camera/viewini.h"
#include "game/car/aipath.h"
#include "game/gfx/poly1.h"
#include "game/math/rotmat.h"
#include "psyq/libgte/gtemat3.h"
#include "psyq/libgte/gtemat7.h"
#include "psyq/libgte/gtetrig.h"
#include "rr/hud_number.h"
#include "game/data/globals.h"
#include "game/gfx/drawbuf.h"

#define G ((Scratch_8006EF8C *)SCRATCHPAD)

struct S8 { int w[8]; };

typedef struct {
    s16 f0;
    s16 f2;
    s16 f4;
    s16 f6;
} S_800A3C20;

typedef struct {
    s32 f00;
    s32 f04;
    s32 pad08[23];
    s32 f64;
    s32 f68;
    u16 f6C;
    u16 f6E;
    u16 f70;
    u16 f72;
} Scratch_8006EF8C;


extern int ScaleMatrix();
extern int ViewSegmentObjectList;
extern struct S8 CameraViewMatrix;
extern struct S8 HalfTransViewMatrix;
extern int SetLightMatrix();
extern M2C_UNK ColorMatrix;
extern char D_800ABF48[];
extern short CourseIndex;
extern void RunCourseDrawChain();
extern unsigned short D_8010E280;
extern unsigned int ViewSegmentObjects;
extern int ApplyMatrix();
extern int RotMatrixYXZ();
extern short LightMatrix[][3];
extern char LightAngles[][8];
extern unsigned short LightDirMagnitude[];
extern s16 ViewOffsetX[];
extern s16 ViewOffsetY[];
extern s32 ViewScreenH[];
extern u16 D_800A3BE0[];
extern s16 D_800A3BF0[];
extern u16 D_800A3C00[];
extern s16 D_800A3C10[];
extern S_800A3C20 D_800A3C20[][7];
extern void SetGeomOffset(s32, s32);
extern void SetGeomScreen(s32);
extern void SetDrawArea(s32);
extern void AddPrim(s32, s32);
extern int ViewUsesCarTrackNode;
extern Car PlayerCars[];

void func_8006E9E8(int a0, int *a1);
void SetupViewMatrix(void);
void SetupViewLighting(void);
void SetupPlayerView(int ignoredPlayer);
void PrepareViewFrame(void);
void PrepareRearViewFrame(void);
void SetGteObjectTransform(short *a0, short *a1, int *a2);
void BuildLightMatrix(void);
void SetupViewport(s32 a0);
int GetViewTrackNode(int *a0, int a1);

void func_8006E9E8(int a0, int *a1) {
    int v[4], w[4], x[4];
    int *g = (int *)(SCRATCHPAD + 0x7C);
    if (a0) {
        BuildRotMatrixAngles();
        v[0] = (0x8000 - a1[0]) << 2;
        v[1] = (0x60 - a1[1]) << 2;
        v[2] = (0x8000 - a1[2]) << 2;
        ApplyMatrixLV((SCRATCHPAD + 0x108), v, w);
        x[2] = 0x4000;
        x[1] = 0x4000;
        x[0] = 0x4000;
        g[0x28] = w[0];
        g[0x29] = w[1];
        g[0x2A] = w[2];
        ScaleMatrix((SCRATCHPAD + 0x108), x);
        func_8006E9D4(ViewSegmentObjectList, a0, a1);
    }
}

void SetupViewMatrix(void) {
    int v[4], w[4], x[4];
    int *h = (int *)SCRATCHPAD;
    int *g = (int *)(SCRATCHPAD + 0x8);
    v[0] = 0x8000 - (g[0] >> 2);
    v[1] = -(g[1] >> 2);
    v[2] = 0x8000 - (g[2] >> 2);
    ApplyMatrixLV(SCRATCH_ROT_MATRIX, v, w);
    CameraViewMatrix.w[5] = w[0];
    CameraViewMatrix.w[6] = w[1];
    CameraViewMatrix.w[7] = w[2];
    HalfTransViewMatrix = CameraViewMatrix;
    v[0] = 0x20000 - g[0];
    v[1] = -g[1];
    v[2] = 0x20000 - g[2];
    HalfTransViewMatrix.w[5] >>= 1;
    HalfTransViewMatrix.w[6] >>= 1;
    HalfTransViewMatrix.w[7] >>= 1;
    ApplyMatrixLV(SCRATCH_ROT_MATRIX, v, w);
    h[15] = w[0];
    h[16] = w[1];
    h[17] = w[2];
    SetTransMatrix(SCRATCH_ROT_MATRIX);
    x[2] = 0x4000;
    x[1] = 0x4000;
    x[0] = 0x4000;
    ScaleMatrix(SCRATCH_ROT_MATRIX, x);
    SetRotMatrix(SCRATCH_ROT_MATRIX);
}

/* legacy typing of shared globals in this part of the unit */
#define LightMatrix (*(M2C_UNK (*))&LightMatrix)

void SetupViewLighting(void) {
    SetColorMatrix(&ColorMatrix);
    BuildLightMatrix();
    SetLightMatrix(&LightMatrix);
    SetBackColor(BackColorR, BackColorG, BackColorB);
    SetFarColor(FarColorR, FarColorG, FarColorB);
    SetFogNear(0x4E20, 0x122);
}

#undef LightMatrix

void SetupPlayerView(int ignoredPlayer) {
    char *p = D_800ABF48;
    BuildViewSegmentObjectList();
    func_80071348();
    SetupViewLighting();
    func_8006E9E8(p, p - 0x30);
    SetupViewMatrix();
    RunCourseDrawChain(SCRATCHPAD, ViewSegmentObjectList, CourseIndex);
}

/* legacy typing of shared globals in this part of the unit */
#define RunCourseDrawChain ((int (*)())RunCourseDrawChain)
#define BuildViewSegmentObjectList ((int (*)())BuildViewSegmentObjectList)
#define func_80071348 ((int (*)())func_80071348)

void PrepareViewFrame(void) {
    BuildViewSegmentObjectList();
    AddUniqueViewSegmentRecords();
    func_80071348();
    SetupViewLighting();
    SetupViewMatrix();
    RunCourseDrawChain(SCRATCHPAD, ViewSegmentObjectList, CourseIndex);
}

#undef RunCourseDrawChain
#undef BuildViewSegmentObjectList
#undef func_80071348

/* legacy typing of shared globals in this part of the unit */
#define BuildViewSegmentObjectList ((int (*)())BuildViewSegmentObjectList)
#define RunCourseDrawChain ((int (*)())RunCourseDrawChain)

void PrepareRearViewFrame(void) {
    short *g = (short *)SCRATCHPAD;
    unsigned int v; int pad[12];
    D_8010E280 = (unsigned short)g[0x2E];
    g[0x2E] = -1;
    BuildViewSegmentObjectList();
    v = GetOctantTableValue();
    if (v < ViewSegmentObjects) {
        ViewSegmentObjects = v;
    }
    SetupViewLighting();
    SetupViewMatrix();
    RunCourseDrawChain(g, ViewSegmentObjectList, CourseIndex);
    g[0x2E] = (short)D_8010E280;
}

#undef BuildViewSegmentObjectList
#undef RunCourseDrawChain

void SetGteObjectTransform(short *a0, short *a1, int *a2) {
    short *g = (short *)SCRATCHPAD;
    a0[0] = a1[0] - g[4];
    a0[1] = a1[2] - g[6];
    a0[2] = a1[4] - g[8];
    ApplyMatrix(SCRATCH_ROT_MATRIX, a0, a0 + 4);
    *(int *)(a0 + 0x16) = *(int *)(a0 + 4);
    *(int *)(a0 + 0x18) = *(int *)(a0 + 6);
    *(int *)(a0 + 0x1A) = *(int *)(a0 + 8);
    SetRotMatrix(a2);
    SetTransMatrix(a0 + 12);
}

void BuildLightMatrix(void) {
    int i;
    int m[8];
    struct { short a, b, c, d; } s;
    struct { short x, p0, y, p1, z, p2, q[2]; } r;
    s.b = 0;
    s.a = 0;
    for (i = 0; i < 3; i++) {
        s.c = -LightDirMagnitude[i];
        RotMatrixYXZ(LightAngles[i], m);
        SetRotMatrix(m);
        ApplyRotMatrix(&s, &r);
        LightMatrix[i][0] = r.x;
        LightMatrix[i][1] = r.y;
        LightMatrix[i][2] = r.z;
    }
}

void SetupViewport(s32 a0) {
    /* ASM_BARRIER: Keep scratchpad base pointer in $s2 initialized in prologue. */
    register Scratch_8006EF8C *s2 asm("$18") = G;
    /* ASM_BARRIER: Keep s3 in $s3 so s3 += 0xC modifies $s3 in place before storing to SCRATCHPAD. */
    register s32 s3 asm("$19");
    s32 s1;
    s32 s5;
    s32 v1;
    S_800A3C20 *p;
    /* ASM_BARRIER: Keep a0_val in $a0 in the s5 != 0 branch so p is allocated to $a1. */
    register s32 a0_val asm("$4");
    /* ASM_BARRIER: Keep a2_val in $a2 in the s5 != 0 branch so p is allocated to $a1. */
    register s32 a2_val asm("$6");

    /* ASM_BARRIER: Materialize $s2 = SCRATCHPAD at start of prologue. */
    __asm__("" : "=r"(s2) : "0"(s2));
    s3 = s2->f00;
    CurrentViewIndex = a0;
    s1 = (a0 == 1);
    SetGeomOffset(ViewOffsetX[a0], ViewOffsetY[a0]);
    s5 = (a0 == 1);
    SetGeomScreen(ViewScreenH[a0]);
    s2->f6C = D_800A3BE0[a0];
    v1 = D_800A3BF0[a0];
    if (s1) {
        v1 += RearViewMirrorOffsetY;
    }
    s2->f6E = v1;
    s2->f70 = D_800A3C00[a0];
    v1 = D_800A3C10[a0];
    if (s1) {
        v1 += RearViewMirrorOffsetY;
    }
    s2->f72 = v1;
    p = &D_800A3C20[DrawBufIndex][a0];
    if (!s5) {
        s2->f64 = 0;
    } else {
        s2->f64 = 1;
        a0_val = RearViewMirrorOffsetY;
        if (a0_val < 0) {
            a0_val = 0;
        }
        p->f2 = a0_val;
        a2_val = s2->f72;
        /* ASM_BARRIER: Prevent combine from folding lhu into lh + duplicate lhu. */
        __asm__("" : "=r"(a2_val) : "0"(a2_val));
        if ((s16)a0_val < (s16)a2_val) {
            a0_val = a2_val - a0_val;
        } else {
            a0_val = 0;
        }
        p->f6 = a0_val;
        v1 = p->f2;
        if (DrawBufIndex != 0) {
            v1 += 0x100;
        }
        p->f2 = v1;
    }
    if (a0 & 1) {
        s2->f04 = CurrentDrawBuf + 0xB70;
    } else {
        s2->f04 = CurrentDrawBuf + 0x70;
    }
    SetDrawArea(s3);
    AddPrim(s2->f04 + 0xAFC, s3);
    s3 += 0xC;
    G->f00 = s3;
}

int GetViewTrackNode(int *a0, int a1) {
    if (ViewUsesCarTrackNode == 0) {
        return FindTrackNodeAtXZ(a0[0], a0[2]);
    }
    return PlayerCars[a1].wF0;
}
