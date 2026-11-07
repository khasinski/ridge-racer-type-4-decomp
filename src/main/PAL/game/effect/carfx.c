#include "game/car/car.h"
#include "scratchpad.h"
#include "rr/gpu.h"
#include "rr/line_emit.h"
#include "game/camera/camfx.h"
#include "game/gfx/gfxotag.h"
#include "game/effect/spark.h"
#include "game/gfx/drawbuf.h"

typedef struct {
    int vx, vy, vz, pad;
} VECTOR;

typedef struct {
    short vx, vy, vz, pad;
} SVECTOR;

typedef struct {
    short m[3][3];
    short pad;
    int t[3];
} MATRIX;

typedef struct {
    VECTOR vec;
    SVECTOR rot;
    int f18;
    int f1C;
    int f20;
    int f24;
    int f28;
} Node;

typedef struct {
    char pad0[0x108];
    int f108;
    int f10C;
    char pad1[0x138 - 0x110];
    Node (*nodes)[24];
} Sub24C;

typedef struct {
    char pad0[0x60];
    Node sub60;
    char pad1[0x24C - 0x8C];
    Sub24C *p;
} Context650;

typedef struct {
    int pad0[2];
    VECTOR vec08;
    int r18;
    int r1C;
    int r20;
} Scratch;

extern int BuildCameraMatrices();
extern int ApplyMatrix();
extern int SetGeomScreen();
extern int RotMatrix();
extern VECTOR D_800A0FC0;
extern SVECTOR D_800A0FD0;
extern int D_800A0FD8;
extern Car PlayerCars;
extern unsigned char D_800A0DC8[][4];
extern u16 D_800111FE;
extern short *D_800A0F20[];
extern short *D_800A0F8C[];
extern short D_800AD520;
extern int PsyqRand();
extern struct { char pad[0x24C]; struct { char pad[0x140]; int v; } *p; } RaceContext;

int func_8005642C(Car *arg0, int arg1, int arg2, int *arg3);
void DrawHudSprite(int a0, int a1, int a2, int a3, int a4);
void DrawHudDigit();
void func_80057138(int a0, int a1);
void func_800572A8();
void DrawSparkHudOverlay();

/* legacy typing of shared globals in this part of the unit */
#define RaceContext (*(Context650 (*))&RaceContext)

int func_8005642C(Car *arg0, int arg1, int arg2, int *arg3) {
    VECTOR sp10;
    VECTOR sp20;
    VECTOR sp30;
    MATRIX sp40;
    SVECTOR sp60;
    SVECTOR sp68;
    VECTOR sp70;
    Scratch *s5 = (Scratch *)SCRATCHPAD;
    int s0 = -1;
    int s3;
    int i;
    Node *s6;
    int s2;
    int s4;

    s6 = RaceContext.p->nodes[arg2];
    for (i = 1; i < 0x18; i++) {
        if (s6[i].f24 * 10 >= arg1) {
            s0 = i;
            s3 = i - 1;
            *arg3 = s3;
            break;
        }
    }

    if (s0 < 0) {
        sp60.vx = 0;
        sp60.vy = -0xA00;
        sp60.vz = 0;
        sp68.vx = D_800A0FC0.vx;
        sp68.vy = D_800A0FC0.vy;
        sp68.vz = D_800A0FC0.vz;
        RotMatrix(&sp60, &sp40);
        ApplyMatrix(&sp40, &sp68, &sp70);
        sp70.vx += PlayerCars.u20.s.w20 * 4;
        sp70.vy += PlayerCars.u20.s.w24 * 4;
        sp70.vz += PlayerCars.u20.s.w28 * 4;
        s5->vec08.vx = sp70.vx;
        s5->vec08.vy = sp70.vy;
        s5->vec08.vz = sp70.vz;
        s5->r18 = D_800A0FD0.vx;
        s5->r1C = D_800A0FD0.vy;
        s5->r20 = D_800A0FD0.vz;
        SetGeomScreen(D_800A0FD8);
        BuildCameraMatrices();
        return 1;
    }

    if (s3 != RaceContext.p->f10C) {
        RaceContext.p->f10C = s3;
        if (s6[s3].f28 != 0) {
            RaceContext.p->f108 = s6[s3].f28 * 10;
        }
    }

    func_80056258(RaceContext.p->f108);

    s2 = (s6[s0].f24 - s6[s3].f24) * 10;
    s4 = arg1 - s6[s3].f24 * 10;

    if (s6[s3].f18 == 0) {
        sp20.vx = arg0->u20.s.w20 * 4;
        sp20.vy = arg0->u20.s.w24 * 4;
        sp20.vz = arg0->u20.s.w28 * 4;
    } else {
        sp20 = s6[s3].vec;
    }

    if (s6[s0].f18 == 0) {
        sp30.vx = arg0->u20.s.w20 * 4;
        sp30.vy = arg0->u20.s.w24 * 4;
        sp30.vz = arg0->u20.s.w28 * 4;
    } else {
        sp30 = s6[s0].vec;
    }

    if (s6[s0].f24 < 0x50) {
        if (s2 > 0) {
            RaceContext.sub60.vec.vx = ((sp30.vx - sp20.vx) * s4) / s2 + sp20.vx;
            RaceContext.sub60.vec.vy = ((sp30.vy - sp20.vy) * s4) / s2 + sp20.vy;
            RaceContext.sub60.vec.vz = ((sp30.vz - sp20.vz) * s4) / s2 + sp20.vz;
            RaceContext.sub60.rot.vx = ((s6[s0].rot.vx - s6[s3].rot.vx) * s4) / s2 + s6[s3].rot.vx;
            RaceContext.sub60.rot.vy = ((s6[s0].rot.vy - s6[s3].rot.vy) * s4) / s2 + s6[s3].rot.vy;
            RaceContext.sub60.rot.vz = ((s6[s0].rot.vz - s6[s3].rot.vz) * s4) / s2 + s6[s3].rot.vz;
            RaceContext.sub60.f1C = ((s6[s0].f1C - s6[s3].f1C) * s4) / s2 + s6[s3].f1C;
            RaceContext.sub60.f20 = ((s6[s0].f20 - s6[s3].f20) * s4) / s2 + s6[s3].f20;
        } else {
            RaceContext.sub60.vec = sp30;
            RaceContext.sub60.rot = s6[s0].rot;
            RaceContext.sub60.f1C = s6[s0].f1C;
            RaceContext.sub60.f20 = s6[s0].f20;
        }
        ApplyCameraState(&RaceContext.sub60);
        return 0;
    }

    sp60.vx = 0;
    sp60.vy = -0xA00;
    sp60.vz = 0;
    sp68.vx = D_800A0FC0.vx;
    sp68.vy = D_800A0FC0.vy;
    sp68.vz = D_800A0FC0.vz;
    RotMatrix(&sp60, &sp40);
    ApplyMatrix(&sp40, &sp68, &sp70);
    sp70.vx += PlayerCars.u20.s.w20 * 4;
    sp70.vy += PlayerCars.u20.s.w24 * 4;
    sp70.vz += PlayerCars.u20.s.w28 * 4;

    if (s2 > 0) {
        RaceContext.sub60.vec.vx = sp20.vx;
        RaceContext.sub60.vec.vy = sp20.vy;
        RaceContext.sub60.vec.vz = sp20.vz;
        RaceContext.sub60.rot.vx = s6[s3].rot.vx;
        RaceContext.sub60.rot.vy = s6[s3].rot.vy;
        RaceContext.sub60.rot.vz = s6[s3].rot.vz;
        RaceContext.sub60.f1C = s6[s3].f1C;
        RaceContext.sub60.f20 = s6[s3].f20;
        ApplyCameraState(&RaceContext.sub60);
        RaceContext.sub60.vec = s5->vec08;
        RaceContext.sub60.rot.vx = s5->r18;
        RaceContext.sub60.rot.vy = s5->r1C;
        RaceContext.sub60.rot.vz = s5->r20;
        s5->vec08.vx = ((sp70.vx - RaceContext.sub60.vec.vx) * s4) / s2 + RaceContext.sub60.vec.vx;
        s5->vec08.vy = ((sp70.vy - RaceContext.sub60.vec.vy) * s4) / s2 + RaceContext.sub60.vec.vy;
        s5->vec08.vz = ((sp70.vz - RaceContext.sub60.vec.vz) * s4) / s2 + RaceContext.sub60.vec.vz;
        s5->r18 = ((D_800A0FD0.vx - RaceContext.sub60.rot.vx) * s4) / s2 + RaceContext.sub60.rot.vx;
        s5->r1C = ((D_800A0FD0.vy - RaceContext.sub60.rot.vy) * s4) / s2 + RaceContext.sub60.rot.vy;
        s5->r20 = ((D_800A0FD0.vz - RaceContext.sub60.rot.vz) * s4) / s2 + RaceContext.sub60.rot.vz;
        RaceContext.sub60.f20 = ((D_800A0FD8 - s6[s3].f20) * s4) / s2 + s6[s3].f20;
        SetGeomScreen(RaceContext.sub60.f20);
    } else {
        s5->vec08.vx = sp70.vx;
        s5->vec08.vy = sp70.vy;
        s5->vec08.vz = sp70.vz;
        s5->r18 = D_800A0FD0.vx;
        s5->r1C = D_800A0FD0.vy;
        s5->r20 = D_800A0FD0.vz;
        SetGeomScreen(D_800A0FD8);
    }
    BuildCameraMatrices();
    return 0;
}

#undef RaceContext

void DrawHudSprite(int a0, int a1, int a2, int a3, int a4)
{
  unsigned short sp[2];
  unsigned char *t0;
  unsigned char *p;
  int *g;
  register char *d asm("$5");
  register unsigned lo asm("$6");
  unsigned hi;
  unsigned cmd;
  unsigned char new_var;
  t0 = D_800A0DC8[a0];
  if (a4 != 0)
  {
    sp[0] = 0xA0 - (t0[2] >> 1);
    sp[1] = a2;
  }
  else
  {
    sp[0] = a1;
    sp[1] = a2;
  }
  g = (int *) SCRATCHPAD;
  p = *((unsigned char **) g);
  p[3] = 4;
  p[7] = 0x66;
  p[4] = a3;
  p[5] = a3;
  p[6] = a3;
  *((short *) (p + 8)) = sp[0];
  *((short *) (p + 0xA)) = sp[1];
  *((short *) (p + 0x10)) = t0[2];
  *((short *) (p + 0x12)) = t0[3];
  p[0xC] = t0[0];
  hi = 0xFF000000;
  new_var = t0[1];
  *((short *) (p + 0xE)) = 0x7F38;
  p[0xD] = new_var;
  lo = 0xFFFFFF;
  cmd = 0xE1000229;
  d = CurrentDrawBuf;
  *((unsigned int *) p) = ((*((unsigned int *) p)) & hi) | ((*((unsigned int *) (d + 0x70))) & lo);
  *((unsigned int *) (d + 0x70)) = ((*((unsigned int *) (d + 0x70))) & hi) | (((unsigned int) p) & lo);
  p += 0x14;
  *g = (int) p;
  p[3] = 1;
  d = CurrentDrawBuf;
  *((unsigned int *) (p + 4)) = cmd;
  *((unsigned int *) p) = ((*((unsigned int *) p)) & hi) | ((*((unsigned int *) (d + 0x70))) & lo);
  *((unsigned int *) (d + 0x70)) = ((*((unsigned int *) (d + 0x70))) & hi) | (((unsigned int) p) & lo);
  p += 8;
  *g = (int) p;
}

void DrawHudDigit(a0, a1, a2, a3) s32 a0; s32 a1; s32 a2; s32 a3; {
    RrSpritePacketSignedClut *p;
    s32 k;
    p = *(RrSpritePacketSignedClut **)SCRATCHPAD;
    k = 0x66;
    p->len = 4;
    p->code = k;
    p->r = a3; p->g = a3; p->b = a3;
    p->y = a2;
    p->u = a0 * 8;
    p->v = 0;
    p->w = 8;
    p->h = 0x10;
    p->x = a1;
    p->clut = 0x7F38;
    p->addr = ((RrSpritePacketSignedClut *)(CurrentDrawBuf + 0x70))->addr;
    *(u32 *)(CurrentDrawBuf + 0x70) = (*(u32 *)(CurrentDrawBuf + 0x70) & 0xFF000000) | ((u32)p & 0xFFFFFF);
    p++;
    *(RrSpritePacketSignedClut **)SCRATCHPAD = p;
    p->len = 1;
    *((u32 *)p + 1) = (D_800111FE & 0x9FF) | 0xE1000200;
    p->addr = ((RrSpritePacketSignedClut *)(CurrentDrawBuf + 0x70))->addr;
    *(u32 *)(CurrentDrawBuf + 0x70) = (*(u32 *)(CurrentDrawBuf + 0x70) & 0xFF000000) | ((u32)p & 0xFFFFFF);
    *(RrSpritePacketSignedClut **)SCRATCHPAD = (RrSpritePacketSignedClut *)((u32 *)p + 2);
}

void func_80057138(int a0, int a1) {
    unsigned short sp[2];
    int s0, s1;
    int s2;
    int s3;
    int d1, d2, d3, d4;
    s2 = a0;
    if (s2 < 0x10) {
        s0 = 0x10 - s2;
        sp[0] = 0x80 - s0;
    } else {
        s0 = 0;
        sp[0] = 0x80;
    }
    sp[1] = 0xD8;
    if (a1 < s2) {
        s1 = 0x80 - ((s2 - a1) * 4);
    } else {
        s1 = 0x80;
    }
    s3 = 0;
    d1 = s0 + 0x10;
    d2 = s0 - 0x36;
    d3 = s0 - 0x10;
    d4 = s0 + 0x36;
    do {
        DrawHudSprite(0, (short)(sp[0] - d1), (short)(sp[1] + d2), s1, 0);
        DrawHudSprite(1, (short)(sp[0] + d3), (short)(sp[1] - d4), s1, 0);
        s3++;
        sp[0] += 0x20;
        sp[0] += s0;
    } while (s3 < 3);
    if (s2 >= 0x10) {
        s2 -= 0x10;
        if (s2 < 0x10) s1 = s2 * 8;
        DrawHudSprite(2, 0, 0x110, s1, 1);
    }
}

void func_800572A8(a0, a1)
int a0;
int a1;
{
  s16 v[4];
  u8 w[3];
  short *s1;
  int i;
  int c;
  unsigned short ua;
  s1 = D_800A0F20[*((int *) (SparkCarState + 0x13C))];
  do { c = a0 * 4;
  if (a0 >= 0x20) {
    if (a1 < a0) c = 0x80 - ((a0 - a1) * 4);
    else c = 0x80;
  } } while (0);
  i = 0; v[0] = 0x7C; v[2] = 0xC4; v[3] = 0xB0; v[1] = 0xB0; w[2] = c; w[1] = c; w[0] = c;
  do { AddLineF2Prim(v, w, 1, 0); i += 1; v[1] += 1; v[3] += 1; } while (i < 2);
  i = 0; v[3] = 0x118; v[1] = 0x118; w[2] = c; w[1] = c; w[0] = c;
  do { AddLineF2Prim(v, w, 1, 0); i += 1; v[1] += 1; v[3] += 1; } while (i < 2);
  ua = s1[2];
  if (s1[2] >= 0) {
    do {
      DrawHudSprite((short) ua, s1[0], s1[1], c, s1[3]);
      s1 += 4;
      ua = s1[2];
    } while (s1[2] >= 0);
  }
  s1 = D_800A0F8C[D_800AD520];
  ua = s1[2];
  if (s1[2] >= 0) {
    do {
      DrawHudSprite((short) ua, s1[0], s1[1], c, s1[3]);
      s1 += 4;
      ua = s1[2];
    } while (s1[2] >= 0);
  }
}

void DrawSparkHudOverlay(a0, a1, a2) int a0, a1, a2; {
    int s4 = 0x80;
    int s5 = 0;
    int s0 = 0;
    int s1 = 0;
    int r, q;
    if (a1 < 0x20) {
        s4 = a1 << 2;
    } else if (a2 < a1) {
        s4 -= (a1 - a2) << 2;
    }
    if (a1 < 0x30) {
        r = PsyqRand();
        q = r / 16;
        q = r - q * 16;
        s0 = q - 8;
        r = PsyqRand();
        q = r / 8;
        q = r - q * 8;
        s1 = q - 4;
        s5 = s4 / 4;
    }
    switch (a0) {
    case 0:
        DrawHudSprite(0x1A, 0, (short)(s0 + 0xD8), s4, 1);
        DrawHudSprite(0x1A, 0, (short)(s1 + 0xD8), s5, 1);
        break;
    case 1:
        DrawHudSprite(0xE, 0, (short)(s0 + 0xD8), s4, 1);
        DrawHudSprite(0xE, 0, (short)(s1 + 0xD8), s5, 1);
        s0 = (short)(s0 + 0xD8);
        s1 = (short)(s1 + 0xD8);
        DrawHudDigit(RaceContext.p->v / 10, 0xA4, s0, s4);
        DrawHudDigit(RaceContext.p->v / 10, 0xA4, s1, s5);
        DrawHudDigit(RaceContext.p->v % 10, 0xA8, s0, s4);
        DrawHudDigit(RaceContext.p->v % 10, 0xA8, s1, s5);
        break;
    case 2:
        DrawHudSprite(0x1B, 0, (short)(s0 + 0xD8), s4, 1);
        DrawHudSprite(0x1B, 0, (short)(s1 + 0xD8), s5, 1);
        break;
    }
}
