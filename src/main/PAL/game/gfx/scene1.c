#include "types.h"
#include "scratchpad.h"
#include "m2c_macros.h"
#include "rr/matrix_words.h"
#include "psyq/libgpu/gpudisp.h"
#include "psyq/libgte/gtemat7.h"
#include "game/menu/menutran.h"
#include "game/gfx/scene1.h"
#include "rr/hud_number.h"
#include "rr/bios_devices.h"
#include "game/gfx/gfxotag.h"
#include "game/gfx/drawbuf.h"

#define addPrim(ot, p) \
    ((DrTPage *)(p))->addr = (ot)->addr; \
    (ot)->addr = (u32)(p)
#define P RaceContext.p
#define B(o) (*(unsigned char *)(P + (o)))
#define H(o) (*(short *)(P + (o)))
#define W(o) (*(int *)(P + (o)))

typedef struct {
    u32 addr : 24;
    u32 len : 8;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    s16 w, h;
} Sprt;

typedef struct {
    u32 addr : 24;
    u32 len : 8;
    u32 code[1];
} DrTPage;

typedef struct {
    u32 addr : 24;
    u32 len : 8;
    u32 code[2];
} DrStp;

typedef struct {
    s16 x, y, w, h;
} Rect;

typedef struct 
{
  char pad[0xB6C];
  unsigned int w[1];
} S;

typedef struct { int w[4]; } Q16;

typedef RrMatrixWords32 M32;

typedef struct { short m[3][3]; short pad[7]; } S70;

extern struct { char pad[0xD4]; s32 a, b, c, d; u8 e, f, g; } RaceContext;
extern s32 D_800AC724;
extern Rect D_800A0D04[];
extern Rect D_800A0D24[][4];
extern int SetModelTexturePage();
extern short D_800A0D7C;
extern int D_800A0FB0[];
extern unsigned short D_800AC006;
extern int DrawSubOtFlag;
extern short CourseListIndex;
extern struct { char pad[8]; short x8; } *CarBoundsTable;
extern Q16 D_800111D4;
extern short LightMatrix[][3];
extern int BuildRotMatrixZ();
extern int BuildRotMatrixY();
extern int BuildRotMatrixX();
extern int MulMatrix0();
extern int MulMatrix();
extern int MulMatrix2();
extern int ScaleMatrix();
extern int SetLightMatrix();
extern int ApplyMatrix();

void func_80055300(void);
void func_80055330(int a0, u8 a1, u8 a2, u8 a3);
void func_8005534C(s32 arg0);
void func_80055358(s32 a0, s32 a1, u8 *a2, s32 a3, s32 a4);
void InitCameraState();
void SetupObjectLightMatrices();
void TransformPoint(int a0, int *a1, u16 *a2, int *a3);

void func_80055300(void) {
    RaceContext.d = 1;
    RaceContext.a = 0;
    RaceContext.b = 0;
    RaceContext.c = 0;
    RaceContext.e = 0x90;
    RaceContext.f = 0x90;
    RaceContext.g = 0x90;
}

/* legacy typing of shared globals in this part of the unit */
#define RaceContext (*(struct { char pad[0xDC]; s32 a; char pad2[4]; u8 b, c, d; } (*))&RaceContext)

void func_80055330(int a0, u8 a1, u8 a2, u8 a3) {
    RaceContext.a = a0;
    RaceContext.b = a1;
    RaceContext.c = a2;
    RaceContext.d = a3;
}

#undef RaceContext

void func_8005534C(s32 arg0) {
    D_800AC724 = arg0;
}

void func_80055358(s32 a0, s32 a1, u8 *a2, s32 a3, s32 a4) {
    s32 i;
    Rect *uv;
    Sprt *s2;
    DrStp *s0;
    s16 y, x;
    /* ASM_BARRIER: Keep SCRATCHPAD in $s1 inside loop without hoisting */
    register DrStp **p1f80 asm("$17");
    s32 base;
    s32 c;
    s32 offset;

    base = (s32)D_800A0D24;
    for (i = 0; i < 4; i++) {
        s2 = *(Sprt **)SCRATCHPAD;
        s2->len = 4;
        c = 0x64;
        /* ASM_BARRIER: Keep 0x64 after s2->len = 4 */
        __asm__ volatile("" : "+r"(c));
        s2->code = c;
        /* ASM_BARRIER: Clobber c before if(a4 != 0) so cse1/cse2 keep c = 0x64 in else-branch */
        __asm__ volatile("" : "+r"(c));
        if (a4 != 0) {
            c = 0x66;
            s2->code = c;
        } else {
            c = 0x64;
            s2->code = c;
        }
        if (a1 != 0) {
            s2->code |= 1;
        } else {
            s2->code &= ~1;
        }
        s2->r0 = a2[0];
        s2->g0 = a2[1];
        s2->b0 = a2[2];
        s2->x0 = D_800A0D04[i].x;
        s2->y0 = D_800A0D04[i].y;
        s2->w = D_800A0D04[i].w;
        s2->h = D_800A0D04[i].h;
        offset = (i << 3) + (a0 << 5);
        uv = (Rect *)(offset + base);
        s2->u0 = uv->x;
        s2->v0 = uv->y;
        addPrim(&DRAWBUF->ot[0], s2);
        s2++;
        s0 = (DrStp *)((DrTPage *)s2 + 1);
        ((DrTPage *)s2)->len = 1;
        y = uv->h;
        x = uv->w;
        ((DrTPage *)s2)->code[0] = 0xE1000200 | (((a3 & 3) << 5) | 0x100 | ((s16)((y & 0x100) >> 4)) | ((x & 0x3FF) >> 6) | ((s16)(y & 0x200) << 2));
        addPrim(&DRAWBUF->ot[0], s2);
        p1f80 = (DrStp **)SCRATCHPAD;
        *p1f80 = s0;
        SetDrawStp(s0, 0);
        addPrim(&DRAWBUF->ot[0], s0);
        *p1f80 = s0 + 1;
        /* ASM_BARRIER: Keep p1f80 multi-set during loop_optimize; deleted as dead by flow */
        __asm__("" : "=r"(p1f80));
    }
}

/* legacy typing of shared globals in this part of the unit */
#define SetDrawStp ((int (*)())SetDrawStp)
#define CurrentDrawBuf ((S *)CurrentDrawBuf)

void QueueDrawStp()
{
  int new_var;
  unsigned int *p = *((unsigned int **) SCRATCHPAD);
  unsigned int *new_var2;
  unsigned int *ot;
  SetDrawStp(p, 1);
  ot = CurrentDrawBuf->w;
  *p = (new_var = ((*p) & 0xFF000000) | ((*ot) & 0xFFFFFF));
  *ot = ((*ot) & 0xFF000000) | (((unsigned int) p) & 0xFFFFFF);
  new_var2 = p;
  *((unsigned int **) SCRATCHPAD) = (p = new_var2 + 3);
}

#undef SetDrawStp
#undef CurrentDrawBuf

/* legacy typing of shared globals in this part of the unit */
#define RaceContext (*(struct { char pad[0xD4]; s32 a, b, c, d; u8 e[4]; } (*))&RaceContext)

void func_80055660(void) {
    if (RaceContext.a != 0) {
        func_80055358(DrawBufIndex, RaceContext.b, RaceContext.e, RaceContext.c, RaceContext.d);
    }
}

#undef RaceContext

/* legacy typing of shared globals in this part of the unit */
#define RaceContext (*(struct { char pad[0x3DC]; s32 a; } (*))&RaceContext)

void func_800556B4(void) {
    if (Pad0Buttons & 0x10) {
        RaceContext.a ^= 1;
        func_8005534C(RaceContext.a);
    }
    func_80055660();
}

#undef RaceContext

/* legacy typing of shared globals in this part of the unit */
#define RaceContext (*(struct { char pad[0xEC]; char buf[0x160]; char *p; } (*))&RaceContext)

void InitCameraState(a0, a1)
int a0, a1;
{
    int t, one;
    char *q;
    P = RaceContext.buf;
    PsyqBzero(P, 0x160);
    H(0x84) = 0x1C1;
    H(0x86) = -0xDF8;
    H(0x88) = -0x79A;
    B(0x78) = 0x80;
    B(0x79) = 0x80;
    B(0x7A) = 0x80;
    H(0x8C) = 0xB9A;
    H(0x8E) = -0xADB;
    H(0x90) = 0x1E2;
    B(0x7C) = 0x40;
    B(0x7D) = 0x40;
    B(0x7E) = 0x40;
    H(0x94) = -0xA4D;
    H(0x96) = -0x642;
    H(0x98) = 0xA86;
    B(0x80) = 0x20;
    B(0x81) = 0x20;
    B(0x82) = 0x20;
    B(0x9C) = 0x40;
    B(0x9D) = 0x40;
    B(0x9E) = 0x40;
    W(0x110) = 0;
    W(0x114) = -0xF0;
    W(0x118) = 0;
    H(0x120) = 0;
    H(0x122) = 0;
    H(0x124) = 0;
    W(0x12C) = 0x800;
    W(0x130) = 0x1B8;
    W(0x18) = -0xC0;
    W(0x30) = 0xC0;
    if (!(D_800A0FB0[CourseListIndex] & ((one = 1) << D_800F4C28))) {
        W(0xA0) = 0xA9;
        W(0xA4) = 0;
    } else {
        W(0xA0) = 0xCA;
        W(0xA4) = 1;
    }
    q = P;
    D_800A0D7C = *(int *)(q + 0xA0) * 4;
    t = *(int *)(q + 0xA0) - 4;
    t = t - (CarBoundsTable->x8 + ((short)D_800AC006 / 2));
    *(int *)(q + 0x100) = t;
    W(0x13C) = a0;
    W(0x140) = a1;
    SetModelTexturePage(0x19, t, &RaceContext, q);
    DrawSubOtFlag = 0;
}

#undef RaceContext

void SetupObjectLightMatrices(a0, a1, a2, a3) char *a0; char *a1; M32 *a2; M32 *a3; {
    M32 s10, s30, s50;
    S70 s70;
    Q16 s90;
    int n;
    s90 = D_800111D4;
    BuildRotMatrixY(&s50, -*(short *)(a0 + 0x12));
    MulMatrix2(SCRATCH_ROT_MATRIX, &s50);
    BuildRotMatrixX(&s10, -*(short *)(a0 + 0x10));
    BuildRotMatrixZ(&s30, -*(short *)(a0 + 0x14));
    MulMatrix(&s10, &s30);
    MulMatrix2(&s50, &s10);
    BuildRotMatrixY(&s50, *(short *)(a0 + 0x12));
    BuildRotMatrixX(&s30, *(short *)(a0 + 0x10));
    MulMatrix(&s50, &s30);
    BuildRotMatrixZ(&s30, *(short *)(a0 + 0x14));
    MulMatrix2(&s50, &s30);
    *a3 = s30;
    n = 0;
    do {
        LightMatrix[n][0] = *(unsigned short *)(a1 + n * 8 + 0xC);
        LightMatrix[n][1] = *(unsigned short *)(a1 + n * 8 + 0xE);
        LightMatrix[n][2] = *(unsigned short *)(a1 + n * 8 + 0x10);
        s70.m[0][n] = (unsigned char)a1[n * 4] * 16;
        s70.m[1][n] = (unsigned char)a1[n * 4 + 1] * 16;
        s70.m[2][n] = (unsigned char)a1[n * 4 + 2] * 16;
        n++;
    } while (n < 3);
    MulMatrix0(LightMatrix, &s10, &s30);
    SetLightMatrix(&s30);
    SetColorMatrix(&s70);
    SetBackColor((unsigned char)a1[0x24], (unsigned char)a1[0x25], (unsigned char)a1[0x26]);
    ScaleMatrix(&s10, &s90);
    *a2 = s10;
}

void TransformPoint(int a0, int *a1, u16 *a2, int *a3) {
    s16 t[3];
    t[0] = a2[0];
    t[1] = a2[2];
    t[2] = a2[4];
    ApplyMatrix(a0, t, a3);
    a3[0] += a1[0];
    a3[1] += a1[1];
    a3[2] += a1[2];
}
