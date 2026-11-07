#include "types.h"
#include "scratchpad.h"
#include "rr/ui.h"
#include "rr/ui_primitives.h"
#include "game/course/trktext.h"
#include "game/effect/ptinit.h"
#include "rr/frame_environment.h"
#include "rr/bios_devices.h"
#include "rr/frame_loop.h"
#include "game/gfx/drawbuf.h"

#define ADDPRIM(ot, p) \
    (p)->addr = ((PolyFT4 *)(ot))->addr; \
    ((PolyFT4 *)(ot))->addr = (u32)(p)

typedef struct { int f0, st, tm; int pad[2]; short s14; short s16; unsigned short u18; unsigned short u1a; short s1c; unsigned short u1e; } E;

typedef struct {
    s32 active00, state04, timer08;
    s16 x0C, y0E;
    RrGpuColorCommand c10;
    s16 s14;
    u16 u16;
    s16 s18, s1A, s1C, s1E;
} RrParticle20;

typedef struct {
    s32 pad00[6];
    s32 val18;
    s32 pad1C[4];
    RrParticle20 e[24];
} RrParticleSys;

typedef struct {
    u32 addr:24;
    u32 len:8;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    s16 x1, y1;
    u8 u1, v1;
    u16 tpage;
    s16 x2, y2;
    u8 u2, v2;
    u16 pad1;
    s16 x3, y3;
    u8 u3, v3;
    u16 pad2;
} PolyFT4;

extern int PsyqRand();
extern RrUiParticle20 ParticleEntries[];
extern unsigned char D_800A0CF4[];
extern RrParticleSys ParticleSystem;
extern struct {
    char pad[0x3E8];
    s32 f3E8;
} RaceContext;
extern RrGpuColorCommand D_800A0CF0;

int FindFreeParticle(void);
void SpawnParticlePair();

/* legacy typing of shared globals in this part of the unit */
#define ParticleSystem (*(struct { s32 a; char p[0x14]; s32 b; } (*))&ParticleSystem)

void InitParticles(void) {
    SetupDrawDispEnvs(0, 0, 0, 3);
    VSyncThreshold = 0x80;
    ParticleSystem.a = 0;
    PsyqBzero(&ParticleSystem, 0x32C);
    ParticleSystem.b = 0x100;
}

#undef ParticleSystem

/* legacy typing of shared globals in this part of the unit */
#define ParticleSystem (*(struct { char pad[0x20]; struct { char p[0xC]; s32 f; char q[0x10]; } e[24]; } (*))&ParticleSystem)

int FindFreeParticle(void) {
    int i;
    for (i = 0; i < 24; i++) {
        if (ParticleSystem.e[i].f == 0) return i;
    }
    return -1;
}

#undef ParticleSystem

void SpawnParticlePair()
{
    RrUiParticle20 *s0;
    unsigned char *s1;
    /* ASM_BARRIER: pin table index to $v1 to match original register allocation */
    register int ix asm("$3");
    unsigned char *tb;
    int v0, r1, r2, r3, r4, r5, r6, one;
    int s5, s2;
    v0 = FindFreeParticle();
    if (v0 >= 0) {
        s0 = &ParticleEntries[v0];
        one = 1;
        s0->active00 = one;
        s0->state04 = 0;
        s0->timer08 = 0;
        r1 = PsyqRand();
        v0 = r1 % 240 - 0x28;
        s0->x0C = v0;
        s5 = (short)v0;
        r2 = PsyqRand();
        s0->spanX14 = 0x168 - (u16)s0->x0C;
        s0->y0E = r2 % 140 + 0xAA;
        r3 = PsyqRand();
        s2 = ((r3 & 3) + one) << one;
        s0->speed16 = s2;
        r4 = PsyqRand();
        ix = r4 - (r4 / 2) * 2; tb = D_800A0CF4; s1 = tb + (ix << 2);
        s0->r10 = s1[0];
        s0->g11 = s1[1];
        s0->b12 = s1[2];
        s0->rot18 = 0;
        s0->velX1C = 0;
        s0->velY1E = 0;
        s0->scale1A = 0x100;
        v0 = FindFreeParticle();
        if (v0 >= 0) {
            s0 = &ParticleEntries[v0];
            s0->active00 = one;
            s0->state04 = 0;
            s0->timer08 = 0;
            r5 = PsyqRand();
            s0->x0C = s5 + r5 % 60 - 0x1E;
            r6 = PsyqRand();
            s0->speed16 = s2;
            s0->spanX14 = 0x168 - (u16)s0->x0C;
            s0->y0E = r6 % 140 + 0xAA;
            s0->r10 = s1[0];
            s0->g11 = s1[1];
            s0->b12 = s1[2];
            s0->rot18 = 0;
            s0->velX1C = 0;
            s0->velY1E = 0;
            s0->scale1A = 0x100;
        }
    }
}

/* legacy typing of shared globals in this part of the unit */
#define ParticleEntries (*(E (*)[])&ParticleEntries)

void UpdateParticle(int a0) {
    E *e = &ParticleEntries[a0];
    int st = e->st;
    int t, d;
    switch (st) {
    case 0:
        if (e->s1c < 0xC) {
            e->s1c = e->s1c + 1;
        }
        e->u18 = e->u18 + e->s1c;
        d = e->s14 - (short)e->u18;
        if (e->s1c * 3 < d) {
            break;
        }
        goto L15;
    case 1:
        if (e->s1c > 0) {
            e->s1c = e->s1c - 2;
            e->u18 = e->u18 + e->s1c;
        } else {
            e->st = 2;
            e->tm = 0x32;
        }
        break;
    case 2:
        e->tm = e->tm - 1;
        if (e->tm <= 0) {
    L15:
            e->st = e->st + 1;
        }
        break;
    case 3:
        t = e->u1a;
        t -= 8;
        e->u1a = t;
        if ((t << 16) <= 0) {
            e->f0 = 0;
        }
        break;
    }
    e->u1e = e->u1e + 1;
}

#undef ParticleEntries

void DrawParticle(int a0) {
    RrGpuColorCommand c[4];
    RrGpuRectangleFields rect;
    RrParticle20 *e;
    PolyFT4 *p;
    int i, v;
    int s3, sp30, s7, s1;

    e = &ParticleSystem.e[a0];
    c[0] = c[1] = c[2] = c[3] = e->c10;
    for (i = 0; i < 4; i++) {
        v = (c[i].r * ParticleSystem.val18) / 256;
        v = (v * RaceContext.f3E8) / 256;
        c[i].r = v;
        v = (c[i].g * ParticleSystem.val18) / 256;
        v = (v * RaceContext.f3E8) / 256;
        c[i].g = v;
        v = (c[i].b * ParticleSystem.val18) / 256;
        v = (v * RaceContext.f3E8) / 256;
        c[i].b = v;
    }
    c[1].r /= 2;
    c[1].g /= 2;
    c[1].b /= 2;
    c[2].r /= 4;
    c[2].g /= 4;
    c[2].b /= 4;
    c[3].r = c[2].r + c[2].r / 2;
    c[3].g = c[2].g + c[2].g / 2;
    c[3].b = c[2].b + c[2].b / 2;
    for (i = 0; i < 4; i++) {
        c[i].r = (c[i].r * e->s1A) / 256;
        c[i].g = (c[i].g * e->s1A) / 256;
        c[i].b = (c[i].b * e->s1A) / 256;
    }

    p = *(PolyFT4 **)SCRATCHPAD;
    if (e->s1E >= 4) {
        p->len = 9;
        p->code = 0x2E;
        p->r0 = c[3].r;
        p->g0 = c[3].g;
        p->b0 = c[3].b;
        p->x0 = e->x0C - 9;
        p->y0 = e->y0E - 0x18;
        p->x1 = e->x0C + 3;
        p->y1 = e->y0E - 0x18;
        p->x2 = e->x0C - 9;
        p->y2 = e->y0E + 0x18;
        p->x3 = e->x0C + 3;
        p->y3 = e->y0E + 0x18;
        p->u0 = ((s16)e->u16 / 2) * 16 + 0x30;
        p->v0 = 0x70;
        p->u1 = ((s16)e->u16 / 2) * 16 + 0x3C;
        p->v1 = 0x70;
        p->u2 = ((s16)e->u16 / 2) * 16 + 0x30;
        p->v2 = 0xA0;
        p->u3 = ((s16)e->u16 / 2) * 16 + 0x3C;
        p->v3 = 0xA0;
        p->tpage = 0xA5;
        p->clut = 0x7EB8;
        ADDPRIM(CurrentDrawBuf + 0x188, p);
        p++;
    }

    p->len = 9;
    p->code = 0x2E;
    p->r0 = c[2].r;
    p->g0 = c[2].g;
    p->b0 = c[2].b;
    p->x0 = e->x0C - 0x10;
    p->y0 = e->y0E - 0x18;
    p->x1 = e->x0C;
    p->y1 = e->y0E - 0x18;
    p->x2 = e->x0C - 0x10;
    p->y2 = e->y0E + 0x18;
    p->x3 = e->x0C;
    p->y3 = e->y0E + 0x18;
    p->u0 = ((s16)e->u16 / 2) * 16 + 0x30;
    p->v0 = 0x70;
    p->u1 = ((s16)e->u16 / 2) * 16 + 0x40;
    p->v1 = 0x70;
    p->u2 = ((s16)e->u16 / 2) * 16 + 0x30;
    p->v2 = 0xA0;
    p->u3 = ((s16)e->u16 / 2) * 16 + 0x40;
    p->v3 = 0xA0;
    p->tpage = 0xA5;
    p->clut = 0x7EB8;
    ADDPRIM(CurrentDrawBuf + 0x188, p);
    p++;

    if (e->s1E >= 5) {
        p->len = 9;
        p->code = 0x2E;
        p->r0 = c[0].r;
        p->g0 = c[0].g;
        p->b0 = c[0].b;
        p->x0 = e->x0C + e->s18 - 0xC;
        p->y0 = e->y0E - 0x18;
        p->x1 = e->x0C + e->s18 + 4;
        p->y1 = e->y0E - 0x18;
        p->x2 = e->x0C + e->s18 - 0xC;
        p->y2 = e->y0E + 0x18;
        p->x3 = e->x0C + e->s18 + 4;
        p->y3 = e->y0E + 0x18;
        p->u0 = ((s16)e->u16 / 2) * 16 + 0x30;
        p->v0 = 0x70;
        p->u1 = ((s16)e->u16 / 2) * 16 + 0x40;
        p->v1 = 0x70;
        p->u2 = ((s16)e->u16 / 2) * 16 + 0x30;
        p->v2 = 0xA0;
        p->u3 = ((s16)e->u16 / 2) * 16 + 0x40;
        p->v3 = 0xA0;
        p->tpage = 0xA5;
        p->clut = 0x7EB8;
        ADDPRIM(CurrentDrawBuf + 0x188, p);
        p++;
    }
    *(PolyFT4 **)SCRATCHPAD = p;
    /* ASM_BARRIER: keep scratchpad packet pointer store before s18 clamp */
    __asm__ volatile("");

    s3 = 0x18;
    if (e->s18 < 0x18) {
        s3 = e->s18;
    }
    sp30 = e->s18 - s3;
    s7 = e->x0C + 3;
    if (e->s1E >= 5) {
        s1 = s7 + sp30;
        rect.x = s1;
        rect.y = e->y0E - (s16)e->u16 / 2 - 0x10;
        rect.w = s3;
        rect.h = 0x10;
        AddSlantedGradientRect(&rect, 0, &D_800A0CF0, &c[2], 0x41);

        rect.x = s1;
        rect.y = e->y0E - (s16)e->u16 / 2;
        rect.w = s3;
        rect.h = (s16)e->u16 / 2;
        AddGouraudRect(&rect, &c[1], &c[2], &c[0], &c[1], 0x41);

        rect.x = s1;
        rect.y = e->y0E;
        rect.w = s3;
        rect.h = (s16)e->u16 / 2;
        AddGouraudRect(&rect, &c[0], &c[1], &c[1], &c[2], 0x41);

        rect.x = s1;
        rect.y = e->y0E + (s16)e->u16 / 2;
        rect.w = s3;
        rect.h = 0x10;
        AddSlantedGradientRect(&rect, 1, &c[2], &D_800A0CF0, 0x41);

        if (sp30 > 0) {
            rect.x = s7;
            rect.y = e->y0E - (s16)e->u16 / 2 - 0x10;
            rect.w = e->s18 - s3;
            rect.h = 0x10;
            AddVerticalGradientRect(&rect, &D_800A0CF0, &c[2], 0x41);

            rect.x = s7;
            rect.y = e->y0E;
            rect.w = e->s18 - s3;
            rect.h = (s16)e->u16 / 2;
            AddVerticalGradientRect(&rect, &c[0], &c[1], 0x41);

            rect.x = s7;
            rect.y = e->y0E - (s16)e->u16 / 2;
            rect.w = e->s18 - s3;
            rect.h = (s16)e->u16 / 2;
            AddVerticalGradientRect(&rect, &c[1], &c[0], 0x41);

            rect.x = s7;
            rect.y = e->y0E + (s16)e->u16 / 2;
            rect.w = e->s18 - s3;
            rect.h = 0x10;
            AddVerticalGradientRect(&rect, &c[2], &D_800A0CF0, 0x41);
        }
    }
}
