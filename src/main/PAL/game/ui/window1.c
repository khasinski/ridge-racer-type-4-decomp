/* RODATA 0x800103c0 0x1c */
#include "scratchpad.h"
#include "rr/gpu.h"
#include "psyq/libgpu/primattr.h"
#include "psyq/libgpu/settrans.h"
#include "rr/race_setup.h"
#include "rr/hud_number.h"
#include "rr/draw_environment.h"
#include "game/gfx/drawbuf.h"

struct P { unsigned addr:24; unsigned len:8; unsigned char r, g, b, code; short x; unsigned short y; unsigned char u, v; unsigned short clut; short w, h; };

extern int AddPrim();
extern char RaceHudPacketsMode0[];
extern char HudSpriteTemplatesType1[];
extern char RaceHudPacketTemplatesB[];
extern char FrameHudSpritePacketsA[];
extern int *AddDrawModePrim();
extern int SetDrawArea();

void InitHudSpritePacket(unsigned char *a0, unsigned char *a1);
void SubmitRaceHudPackets();
void DrawDigitString12();
void DrawDigitString8();
void DrawDigitString16();

void InitHudSpritePacket(unsigned char *a0, unsigned char *a1) {
    a0[3] = 4;
    a0[7] = 0x64;
    *(unsigned short *)(a0 + 8) = *(unsigned short *)(a1 + 0);
    *(unsigned short *)(a0 + 0xA) = *(unsigned short *)(a1 + 2);
    *(unsigned short *)(a0 + 0x10) = *(unsigned short *)(a1 + 4);
    *(unsigned short *)(a0 + 0x12) = *(unsigned short *)(a1 + 6);
    a0[0xC] = a1[8];
    a0[0xD] = a1[0xA];
    *(unsigned short *)(a0 + 0xE) = *(unsigned short *)(a1 + 0xC);
    if (*(int *)(a1 + 0x10) != 0) {
        a0[7] |= 2;
    } else {
        a0[7] &= 0xFD;
    }
    a0[7] |= 1;
}

void InitRaceHudPackets(int a0)
{
    int n, i, j, a, b;
    char *tab;
    char *base;
    char *s1;
    char *s0;
    switch (a0) {
    case 0:
    case 5:
    default:
        n = 4;
        tab = RaceHudPacketsMode0;
        break;
    case 1:
        n = 8;
        tab = HudSpriteTemplatesType1;
        break;
    case 2:
    case 3:
    case 4:
    case 6:
        if (PlayerCount == 2 && a0 == 4) goto l60;
        if (a0 != 6) { n = 12; goto l64; }
    l60:
        n = 6;
    l64:
        tab = RaceHudPacketTemplatesB;
        break;
    }
    for (i = 0; i < n; i++) {
        for (j = 0; j < 2; j++) {
            s1 = FrameHudSpritePacketsA + j * 0x22778;
            s0 = s1 + (i * 0x14 + 0x18);
            InitHudSpritePacket(s0, tab + i * 0x14);
            if (i != 0) AddPrim(s1 + (i * 0x14 + 4), s0);
        }
    }
}

void SubmitRaceHudPackets(a0) int a0;
{
    short sp[4];
    int *s0;
    int a1, d, t;
    int *v;
    int *s1;
    if (a0 == 1) goto L68;
    if (a0 < 2) goto L58;
    if (a0 < 5) goto L7C;
L58:
    a1 = 4;
    s0 = (int *)(CurrentDrawBuf + 0x70);
    goto LC0;
L68:
    a1 = 8;
    s0 = (int *)(CurrentDrawBuf + 0x70);
    goto LC0;
L7C:
    if (PlayerCount != 2 || a0 != 3) {
        if (a0 != 4) goto Lb4;
    }
    a1 = 6;
    s0 = (int *)(CurrentDrawBuf + 0x70);
    goto LC0;
Lb4:
    a1 = 0xC;
    s0 = (int *)(CurrentDrawBuf + 0xB70);
LC0:
    d = CurrentDrawBuf;
    v = (int *)(a1 * 0x14 + d + 0x22674);
    *v = (*v & 0xFF000000) | (*s0 & 0xFFFFFF);
    *s0 = (*s0 & 0xFF000000) | ((d + 0x22688) & 0xFFFFFF);
    s1 = AddDrawModePrim(s0, *(int **)SCRATCHPAD, 5);
    *(int **)SCRATCHPAD = s1;
    if (a0 >= 2) {
        sp[2] = 0x140;
        sp[0] = 0;
        sp[3] = 0x100;
        t = DrawBufIndex; sp[1] = t << 8;
        SetDrawArea(s1, sp);
        *s1 = (*s1 & 0xFF000000) | (*s0 & 0xFFFFFF);
        *s0 = (*s0 & 0xFF000000) | ((int)s1 & 0xFFFFFF);
        *(int **)SCRATCHPAD = s1 + 3;
    }
}

/* legacy typing of shared globals in this part of the unit */
#define CurrentDrawBuf ((char *)CurrentDrawBuf)

void DrawDigitString12(a0, a1, a2, a3)
s32 a0;
s32 a1;
u8 *a2;
s32 a3;
{
    RrSpritePacketSignedClut *p = *(RrSpritePacketSignedClut **)SCRATCHPAD;
    s32 t;
    s32 kb;
    s16 c;
    kb = 0xA8;
    if (*a2 != 0) {
        do {
            s32 ch = *a2;
            a2++;
            p->len = 4;
            p->code = 0x64;
            p->x = a0;
            t = ch - 0x30;
            p->y = a1;
            if (t >= 0xD) {
                c = ch * 8 - 0x14C;
                p->w = 4;
                p->h = 8;
                if (t == 0xF)
                    a0 += 5;
                else
                    a0 += 4;
            } else {
                c = t * 12;
                a0 += 10;
                p->w = 12;
                p->h = 12;
            }
            p->u = c;
            p->v = kb;
            p->code |= 1;
            p->clut = a3;
            p->addr = ((RrSpritePacketSignedClut *)(CurrentDrawBuf + 0x70))->addr;
            ((RrSpritePacketSignedClut *)(CurrentDrawBuf + 0x70))->addr = (u32)p;
            p++;
        } while (*a2 != 0);
    }
    SetDrawMode(p, 0, 1, 5, 0);
    AddPrim(CurrentDrawBuf + 0x70, p);
    *(RrSpritePacketSignedClut **)SCRATCHPAD = (RrSpritePacketSignedClut *)((char *)p + 0xC);
}

#undef CurrentDrawBuf

/* legacy typing of shared globals in this part of the unit */
#define AddDrawModePrim ((s32 (*)())AddDrawModePrim)

void DrawDigitString8(a0, a1, a2, a3, a4)
RrSpritePacket *a0; s32 a1; s32 a2; u8 *a3; s32 a4;
{
    RrSpritePacket *p = *(RrSpritePacket **)SCRATCHPAD;
    s32 c, u;
    s32 kb;
    kb = 0xB4;
    if (*a3 != 0) {
        do {
            c = *a3;
            a3++;
            p->len = 4;
            p->code = 0x64;
            p->x = a1;
            p->y = a2;
            p->w = 8;
            p->h = 8;
            c -= 0x30;
            u = c * 8 + 0x6C;
            if (c >= 10) a1 += 3; else a1 += 7;
            p->u = u;
            p->v = kb;
            p->code |= 1;
            p->clut = a4;
            p->addr = a0->addr;
            a0->addr = (u32)p;
            p++;
        } while (*a3 != 0);
    }
    *(s32 *)SCRATCHPAD = AddDrawModePrim(a0, p, 5);
}

#undef AddDrawModePrim

/* legacy typing of shared globals in this part of the unit */
#define AddDrawModePrim ((int (*)())AddDrawModePrim)

void DrawDigitString16(a0, a1, a2, a3)
struct P *a0; int a1; int a2; unsigned char *a3;
{
    struct P *p = *(struct P **)SCRATCHPAD;
    int c, u, kh;
    if (*a3 != 0) {
        kh = 0x10;
        do {
            c = *a3;
            a3++;
            p->len = 4;
            p->code = 0x64;
            p->x = (short)a1;
            p->y = (unsigned short)a2;
            c -= 0x30;
            u = c * 8;
            if (c < 10) {
                a1 += 5;
                p->w = 8;
                p->h = kh;
            } else {
                a1 += 3;
                p->w = 4;
                p->h = kh;
            }
            SetShadeTex(p, 1);
            SetSemiTrans(p, 1);
            { int v58 = 0x58; p->u = u; p->v = v58; p->clut = 0x7FA4; }
            p->addr = a0->addr;
            a0->addr = (unsigned)p;
            p++;
        } while (*a3 != 0);
    }
    *(int *)SCRATCHPAD = AddDrawModePrim(a0, p, 0x59);
}

#undef AddDrawModePrim
