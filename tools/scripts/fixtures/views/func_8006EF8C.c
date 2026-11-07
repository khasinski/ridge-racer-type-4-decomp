#include "types.h"

#define G ((Scratch_8006EF8C *)0x1F800000)

/* legacy typing of shared globals in this part of the unit */
#define LightMatrix (*(M2C_UNK (*))&LightMatrix)

#undef LightMatrix

/* legacy typing of shared globals in this part of the unit */
#define RunCourseDrawChain ((int (*)())RunCourseDrawChain)

#define BuildViewSegmentObjectList ((int (*)())BuildViewSegmentObjectList)

#define ApplySegmentOctantValue ((int (*)())ApplySegmentOctantValue)

#undef RunCourseDrawChain

#undef BuildViewSegmentObjectList

#undef ApplySegmentOctantValue

/* legacy typing of shared globals in this part of the unit */
#define BuildViewSegmentObjectList ((int (*)())BuildViewSegmentObjectList)

#define RunCourseDrawChain ((int (*)())RunCourseDrawChain)

#undef BuildViewSegmentObjectList

#undef RunCourseDrawChain

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

extern s16 ViewOffsetX[];

extern s16 ViewOffsetY[];

extern s32 ViewScreenH[];

extern u16 ViewClipX[];

extern s16 ViewClipY[];

extern u16 ViewClipRight[];

extern s16 ViewportClipHeights[];

extern S_800A3C20 ViewClipRects[][7];

extern s32 CurrentViewIndex;

extern s32 CurrentDrawBuf;

extern s32 DrawBufIndex;

extern s32 RearViewMirrorOffsetY;

extern void SetGeomOffset(s32, s32);

extern void SetGeomScreen(s32);

extern void SetDrawArea(s32);

extern void AddPrim(s32, s32);

void SetupViewport(s32 a0) {
    /* ASM_BARRIER: Keep scratchpad base pointer in $s2 initialized in prologue. */
    register Scratch_8006EF8C *s2 asm("$18") = G;
    /* ASM_BARRIER: Keep s3 in $s3 so s3 += 0xC modifies $s3 in place before storing to 0x1F800000. */
    register s32 s3 asm("$19");
    s32 s1;
    s32 s5;
    s32 v1;
    S_800A3C20 *p;
    /* ASM_BARRIER: Keep a0_val in $a0 in the s5 != 0 branch so p is allocated to $a1. */
    register s32 a0_val asm("$4");
    /* ASM_BARRIER: Keep a2_val in $a2 in the s5 != 0 branch so p is allocated to $a1. */
    register s32 a2_val asm("$6");

    /* ASM_BARRIER: Materialize $s2 = 0x1F800000 at start of prologue. */
    __asm__("" : "=r"(s2) : "0"(s2));
    s3 = s2->f00;
    CurrentViewIndex = a0;
    s1 = (a0 == 1);
    SetGeomOffset(ViewOffsetX[a0], ViewOffsetY[a0]);
    s5 = (a0 == 1);
    SetGeomScreen(ViewScreenH[a0]);
    s2->f6C = ViewClipX[a0];
    v1 = ViewClipY[a0];
    if (s1) {
        v1 += RearViewMirrorOffsetY;
    }
    s2->f6E = v1;
    s2->f70 = ViewClipRight[a0];
    v1 = ViewportClipHeights[a0];
    if (s1) {
        v1 += RearViewMirrorOffsetY;
    }
    s2->f72 = v1;
    p = &ViewClipRects[DrawBufIndex][a0];
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
