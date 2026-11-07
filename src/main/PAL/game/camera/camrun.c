#include "types.h"
#include "scratchpad.h"
#include "game/camera/cammove.h"
#include "game/camera/campath.h"
#include "game/gfx/motion1.h"
#include "game/gfx/ring2.h"
#include "game/camera/campath.h"
#include "game/gfx/motion1.h"
#include "game/gfx/ring.h"
#include "psyq/libgpu/gpudisp.h"
#include "game/camera/camrun.h"
#include "rr/bios_devices.h"
#include "game/gfx/drawbuf.h"

#define W(p,o) (*(int *)((char *)(p) + (o)))
#define Q(p,o) ((char *)*(volatile int *)((char *)(p) + (o)))

typedef struct {
    s32 v[4];
} Vec4;

typedef struct {
    s16 s0;
    s16 s2;
    s32 w4;
} Entry8;

typedef struct {
    Vec4 v0;
    Vec4 v1;
    char pad20[0x48 - 0x20];
    s32 f48;
    s32 f4c;
    s32 f50;
    s32 f54;
    s32 f58;
    s32 f5c;
    s32 f60;
    Entry8 *f64;
    s16 f68;
    s16 f6a;
    s32 f6c;
    s32 f70;
} CamQ;

typedef struct {
    u32 addr : 24;
    u32 len : 8;
} PTag;

typedef struct {
    u32 addr : 24;
    u32 len : 8;
    u32 code;
    u32 p2;
} DrStp;

typedef struct {
    u32 addr : 24;
    u32 len : 8;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    s16 w, h;
} TilePrim;

typedef struct {
    void *p00;
    s32 p04;
    Vec4 v08;
    Vec4 v18;
    char pad28[0x60 - 0x28];
    s32 f60;
} Scratch5B304;

extern struct {
    char pad[0x3D8];
    CamQ *q;
} RaceContext;
extern s32 VSyncThreshold;
extern s16 D_800F7488;
extern s16 D_800F7624;
extern void ScaleMatrixRow1(s32, s32);
extern void UpdateMotionTrack(s16 *);
extern void UpdateCameraMotion(s16 *);
extern void UpdateMotionControllerPanel(s16 *);
extern void UpdateScrollTrack(s16 *);
extern void BuildCameraMatrices(void);
extern void SetupViewport(s32);
extern void SetGeomOffset(s32, s32);
extern void SetGeomScreen(s32);
extern char D_800A1AA0[];
extern char D_800A1B60[];

void StepCameraScript(void);

void StepCameraScript(void) {
    Scratch5B304 *g = (Scratch5B304 *)SCRATCHPAD;
    s32 a2 = 0;
    s16 *s0;

    if (VSyncThreshold == 0x80) {
        RaceContext.q->f4c = 2;
    } else {
        RaceContext.q->f4c = 1;
    }
    if (RaceContext.q->f5c == 0) {
        s0 = &D_800F7624;
    } else {
        s0 = &D_800F7488;
    }
    if (*s0 != 0) {
        if (++RaceContext.q->f6a >= RaceContext.q->f64[RaceContext.q->f68].w4) {
            RaceContext.q->f68++;
            if (RaceContext.q->f64[RaceContext.q->f68].s0 < 0) {
                RaceContext.q->f68 = 0;
            }
            RaceContext.q->f6c = RaceContext.q->f64[RaceContext.q->f68].s0;
            RaceContext.q->f70 = RaceContext.q->f64[RaceContext.q->f68].s2;
            a2 = 1;
            RaceContext.q->f6a = 0;
        }
    }
    switch (RaceContext.q->f6c) {
    case 0:
        if (a2 != 0) {
            InitCameraMotion(RaceContext.q->f70);
        }
        UpdateCameraMotion(s0);
        break;
    case 1:
        if (a2 != 0) {
            InitMotionControllerPanel(RaceContext.q->f70);
        }
        UpdateMotionControllerPanel(s0);
        break;
    case 2:
        if (a2 != 0) {
            InitScrollTrack(RaceContext.q->f70);
        }
        UpdateScrollTrack(s0);
        break;
    case 3:
        if (a2 != 0) {
            InitMotionTrack(RaceContext.q->f70);
        }
        UpdateMotionTrack(s0);
        break;
    }
    if (RaceContext.q->f54 != 0) {
        g->v08 = RaceContext.q->v0;
        g->v18 = RaceContext.q->v1;
        BuildCameraMatrices();
        SetGeomScreen(RaceContext.q->f48);
        if (RaceContext.q->f50 == 0) {
            SetupViewport(6);
            SetGeomOffset(0xA0, 0xE8);
            ScaleMatrixRow1(2, SCRATCH_ROT_MATRIX);
        }
        g->f60 = 5;
        g->p04 = (s32)&DRAWBUF->tailB70;
        if (RaceContext.q->f54 != 0) {
            switch (RaceContext.q->f6c) {
            case 0:
                DrawCameraMotionModel();
                break;
            case 1:
                DrawMotionControllerPanel();
                break;
            case 2:
                DrawRingEffects();
                break;
            case 3:
                DrawMarkerBitmapGrid();
                break;
            }
        }
        if (RaceContext.q->f58 == 0) {
            ((DrStp *)g->p04)->addr = DRAWBUF->tailB68.addr;
            DRAWBUF->tailB68.addr = g->p04 + 0xAFC;
        } else {
            DrStp *s5 = ((Scratch5B304 *)SCRATCHPAD)->p00;
            DrStp *s0_p;

            SetDrawStp(s5, 0);
            s5->code |= 2;
            s5->addr = DRAWBUF->tailB6C.addr;
            DRAWBUF->tailB6C.addr = (u32)s5;
            s5++;
            ((Scratch5B304 *)SCRATCHPAD)->p00 = s5;
            ((DrStp *)g->p04)->addr = DRAWBUF->tailB6C.addr;
            DRAWBUF->tailB6C.addr = g->p04 + 0xAFC;
            s5 = ((Scratch5B304 *)SCRATCHPAD)->p00;
            SetDrawStp(s5, 0);
            s5->addr = DRAWBUF->tailB6C.addr;
            DRAWBUF->tailB6C.addr = (u32)s5;
            s5++;
            ((Scratch5B304 *)SCRATCHPAD)->p00 = s5;
            ((TilePrim *)s5)->len = 3;
            ((TilePrim *)s5)->code = 0x60;
            ((TilePrim *)s5)->r0 = 0;
            ((TilePrim *)s5)->g0 = 0;
            ((TilePrim *)s5)->b0 = 0;
            ((TilePrim *)s5)->x0 = 0;
            ((TilePrim *)s5)->y0 = 0;
            ((TilePrim *)s5)->w = 0x140;
            ((TilePrim *)s5)->h = 0xF0;
            s5->addr = DRAWBUF->tailB6C.addr;
            DRAWBUF->tailB6C.addr = (u32)s5;
            s0_p = (DrStp *)((TilePrim *)s5 + 1);
            ((Scratch5B304 *)SCRATCHPAD)->p00 = s0_p;
            SetDrawStp(s0_p, 1);
            __asm__ volatile("" : "+r"(s0_p)); /* ASM_BARRIER: prevent CSE from folding s0_p + 1 into s5 + 28 */
            s5 = s0_p + 1;
            s0_p->addr = DRAWBUF->tailB6C.addr;
            DRAWBUF->tailB6C.addr = (u32)s0_p;
            ((Scratch5B304 *)SCRATCHPAD)->p00 = s5;
        }
        g->p04 = (s32)&DRAWBUF->tailB70;
    }
}

/* legacy typing of shared globals in this part of the unit */
#define RaceContext (*(char (*)[])&RaceContext)

void InitCameraMotion(int a0) {
    char *s1 = RaceContext;
    char *a;
    register char *t asm("$2");
    int v;
    char *t0;
    PsyqBzero(Q(s1, 0x3D8) + 0x74, 0x70);
    if (a0 < 4) {
        W(Q(s1, 0x3D8), 0x78) = 0;
        W(Q(s1, 0x3D8), 0x80) = (int)(D_800A1AA0 + a0 * 0x30);
    } else {
        a0 -= 4;
        W(Q(s1, 0x3D8), 0x78) = 1;
        W(Q(s1, 0x3D8), 0x80) = (int)(D_800A1B60 + (a0 << 5));
    }
    a = RaceContext;
    t0 = Q(a, 0x3D8);
    W(t0, 0xE0) = 0x2D0;
    t = Q(a, 0x3D8);
    v = W(W(t, 0x80), 0);
    W(t, 0xB4) = v;
    W(t, 0x84) = v;
    t = Q(a, 0x3D8);
    v = W(W(t, 0x80), 4);
    W(t, 0xB8) = v;
    W(t, 0x88) = v;
    t = Q(a, 0x3D8);
    v = W(W(t, 0x80), 8);
    W(t, 0xBC) = v;
    W(t, 0x8C) = v;
    t0 = Q(a, 0x3D8);
    W(t0, 0x7C) = 1;
}

#undef RaceContext
