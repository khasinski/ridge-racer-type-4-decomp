/* RODATA 0x800103e0 0x1c */
/* CC1_FLAGS: -fno-strength-reduce */
#include "types.h"
#include "scratchpad.h"
#include "rr/hud_rotating_quad.h"
#include "rr/trig.h"
#include "rr/hud_number.h"
#include "m2c_macros.h"
#include "game/ui/window2.h"
#include "game/ui/window1.h"
#include "rr/race_setup.h"
#include "game/gfx/drawbuf.h"

struct W3G0A { char *p0; char *p4; short s8, sA, sC, sE, s10, s12; char *p14; char *p18; short s1C, s1E, s20, s22, s24, s26; };

extern char AltOrderingTableA[];
extern char AltOrderingTableB[];
extern char RaceSubOtsA[];
extern char RaceOrderingTableB[];
extern int AddDrawModePrim();
extern char HudFrameSpriteTemplates[];
extern char HudFrameSpriteDefsMulti[];

void DrawHudFrameSprites();
void DrawHudRotatingQuad(s32 phase,s32 index);
void *EmitHudGlyph12(u32 *arg0, void *arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5);
void *EmitHudGlyph12Alt(s32 *arg0, void *arg1, s16 arg2, s16 arg3, s32 arg4, s32 arg5);
void DrawHudNumber3(int a0, int a1);

/* legacy typing of shared globals in this part of the unit */
#define HudNumberTargets (*(struct W3G0A (*))&HudNumberTargets)

void InitHudNumberTargets(int a0) {
    switch (a0) {
    case 0: case 1: case 5: break;
    case 2: case 3: case 4: case 6:
        if (a0 == 4 && PlayerCount == 2) {
        } else if (a0 != 6) {
            goto w3g0b;
        }
    }
    HudNumberTargets.p0 = AltOrderingTableA;
    HudNumberTargets.p4 = AltOrderingTableB;
    HudNumberTargets.s8 = 0x106;
    HudNumberTargets.sA = 0xE6;
    HudNumberTargets.sC = 0x122;
    HudNumberTargets.sE = 0xDC;
    HudNumberTargets.s10 = 0x112;
    HudNumberTargets.s12 = 0xCE;
    return;
w3g0b:
    HudNumberTargets.p0 = RaceSubOtsA;
    HudNumberTargets.p4 = RaceOrderingTableB;
    HudNumberTargets.s8 = 0xFC;
    HudNumberTargets.sA = 0x58;
    HudNumberTargets.sC = 0x128;
    HudNumberTargets.sE = 0x4A;
    HudNumberTargets.s10 = 0xFC;
    HudNumberTargets.s12 = 0x68;
    HudNumberTargets.p14 = RaceSubOtsA;
    HudNumberTargets.p18 = RaceOrderingTableB;
    HudNumberTargets.s1C = 0xFC;
    HudNumberTargets.s1E = 0xD8;
    HudNumberTargets.s20 = 0x122;
    HudNumberTargets.s22 = 0xCA;
    HudNumberTargets.s24 = 0xFC;
    HudNumberTargets.s26 = 0xE8;
}

#undef HudNumberTargets

void DrawHudFrameSprites(a0)
int a0;
{
    unsigned *s0;
    unsigned *s2;
    char *s1;
    char *s4;
    int s3, s5, c3;
    unsigned m1;
    unsigned h1;
    unsigned m2;
    unsigned h2;

    s0 = *(unsigned **)SCRATCHPAD;
    if (a0 >= 2) {
        s2 = (unsigned *)(CurrentDrawBuf + 0xB70);
        s5 = 4;
        if (a0 == 3) s5 = 2;
        s3 = 0;
        if (s5 != 0) {
            m1 = 0xFFFFFF;
            h1 = 0xFF000000;
            s1 = HudFrameSpriteDefsMulti;
            do {
                InitHudSpritePacket(s0, s1);
                s1 += 0x14;
                s3++;
                *s0 = (*s0 & h1) | (*s2 & m1);
                *s2 = (*s2 & h1) | ((unsigned)s0 & m1);
                s0 += 5;
            } while (s3 < s5);
        }
    } else {
        s3 = 0;
        m2 = 0xFFFFFF;
        h2 = 0xFF000000;
        s4 = HudFrameSpriteTemplates;
        s2 = (unsigned *)(CurrentDrawBuf + 0x70);
        s1 = (char *)s0 + 0xA;
        do {
            InitHudSpritePacket(s0, s4);
            __asm__ volatile("" : "=r"(c3) : "0"(3)); /* ASM_BARRIER: keep li 3 inside the loop (not hoisted) */
            if (a0 == c3) *(unsigned short *)s1 -= 0x80;
            s1 += 0x14;
            s4 += 0x14;
            s3++;
            *s0 = (*s0 & h2) | (*s2 & m2);
            *s2 = (*s2 & h2) | ((unsigned)s0 & m2);
            s0 += 5;
        } while (s3 < 3);
    }
    *(int *)SCRATCHPAD = AddDrawModePrim(s2, s0, 5);
}

void DrawHudRotatingQuad(s32 phase,s32 index) {
    RrHudQuadColor colors[4];
    s32 angle,sine;
    s32 cosine;
    s32 i;
    RrHudQuadColor *color;
    RrHudQuadCorner *corner;
    RrNumberTarget *target;
    u32 *head;
    register RrHudQuadEmission *packet __asm__("$8");
    u8 *output;
    register RrNumberTarget (*base)[] __asm__("$4");
    unsigned long row;
    register u32 x __asm__("$13");
    register u32 y __asm__("$4");
    u32 linkMask;
    colors[0].rgb.r=0xD0;
    colors[0].rgb.g=24;
    colors[0].rgb.b=0x30;
    colors[1].rgb.r=0x68;
    colors[1].rgb.g=12;
    colors[1].rgb.b=24;
    colors[2].rgb.r=0x8A;
    colors[2].rgb.g=0x12;
    colors[2].rgb.b=0x24;
    colors[3].rgb.r=0x34;
    colors[3].rgb.g=6;
    colors[3].rgb.b=12;
    angle=(s32)((u32)phase*0xA00U)/10000+0xE00;
    sine=rsin(angle);
    cosine=rcos(angle);
    i=0;color=colors;corner=HudQuadCorners;
    base=&HudNumberTargets;
    row=(unsigned long)index*sizeof(RrNumberTarget);
    target=(RrNumberTarget *)((u8 *)base+row);
    packet=(RrHudQuadEmission *)RR_HUD_QUAD_SCRATCH->cursor;
    /* Full native-width addresses retain the original sum order. */
    head=*(u32 **)((unsigned long)DrawBufIndex*sizeof(u32 *)+row+(unsigned long)base);
    x=(u16)target->x;y=(u16)target->y;
    output=(u8 *)packet+8;
    packet->r=0xB0;packet->g=0xB0;packet->b=0;
    do {
        *(s16 *)output=(s16)(x+(u32)((s32)((u32)cosine*(u32)(s32)corner->x-(u32)sine*(u32)(s32)corner->y)/4096));
        *(s16 *)(output+2)=(s16)(y+(u32)((s32)((u32)sine*(u32)(s32)corner->x+(u32)cosine*(u32)(s32)corner->y)/4096));
        output+=4;
        /* Keep the initialized color destination after the XY writes. */
        *(RrHudQuadColor *)output=*color;
        color++;corner++;i++;output+=4;
    }while(i<4);
    linkMask=0xFFFFFF;
    packet->tag.bytes.length=8;packet->code=0x38;
    packet->tag.word=(packet->tag.word&0xFF000000)|(*head&linkMask);
    linkMask=(unsigned long)packet&linkMask;
    *head=(*head&0xFF000000)|linkMask;
    RR_HUD_QUAD_SCRATCH->cursor=(u8 *)packet+36;
}

void *EmitHudGlyph12(u32 *arg0, void *arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5) {
    M2C_FIELD(arg1, s8 *, 3) = 4;
    M2C_FIELD(arg1, s8 *, 7) = 0x65;
    M2C_FIELD(arg1, s16 *, 0x10) = 0xC;
    M2C_FIELD(arg1, s16 *, 0x12) = 0x10;
    M2C_FIELD(arg1, s16 *, 8) = arg2;
    M2C_FIELD(arg1, s16 *, 0xA) = arg3;
    M2C_FIELD(arg1, u8 *, 0xD) = 0xF0;
    M2C_FIELD(arg1, s8 *, 0xC) = (s8) (arg4 * 0xC);
    M2C_FIELD(arg1, s16 *, 0xE) = (s16) arg5;
    M2C_FIELD(arg1, s32 *, 0) = (s32) ((M2C_FIELD(arg1, s32 *, 0) & 0xFF000000) | (*arg0 & 0xFFFFFF));
    *arg0 = (*arg0 & 0xFF000000) | ((s32) arg1 & 0xFFFFFF);
    return arg1 + 0x14;
}

void *EmitHudGlyph12Alt(s32 *arg0, void *arg1, s16 arg2, s16 arg3, s32 arg4, s32 arg5) {
    M2C_FIELD(arg1, s8 *, 3) = 4;
    M2C_FIELD(arg1, s8 *, 7) = 0x65;
    M2C_FIELD(arg1, s16 *, 0x10) = 0xC;
    M2C_FIELD(arg1, s16 *, 0x12) = 0x10;
    M2C_FIELD(arg1, s16 *, 8) = arg2;
    M2C_FIELD(arg1, s16 *, 0xA) = arg3;
    M2C_FIELD(arg1, u8 *, 0xD) = 0xF0;
    M2C_FIELD(arg1, s8 *, 0xC) = (s8) ((arg4 * 0xC) + 0x78);
    M2C_FIELD(arg1, s16 *, 0xE) = (s16) arg5;
    M2C_FIELD(arg1, s32 *, 0) = (s32) ((M2C_FIELD(arg1, s32 *, 0) & 0xFF000000) | (*arg0 & 0xFFFFFF));
    *arg0 = (*arg0 & 0xFF000000) | ((s32) arg1 & 0xFFFFFF);
    return arg1 + 0x14;
}

/* legacy typing of shared globals in this part of the unit */
#define HudNumberTargets (*(char (*)[])&HudNumberTargets)

void DrawHudNumber3(int a0, int a1)
{
    char *b = HudNumberTargets;
    int off = a1 * 0x14;
    char *t = b + off;
    short x = *(short *)(t + 8);
    short y = *(short *)(t + 10);
    int s1 = *(int *)(DrawBufIndex * 4 + off + (int)b);
    int h, tn, on;
    h = a0 / 100;
    tn = (a0 / 10) - h * 10;
    on = a0 % 10;
    *(int *)SCRATCHPAD = AddDrawModePrim(s1, EmitHudGlyph12(s1, EmitHudGlyph12(s1, EmitHudGlyph12(s1, *(int *)SCRATCHPAD, x, y, h, 0x7EFA), x + 8, y, tn, 0x7EFA), x + 0x10, y, on, 0x7EFA), 5);
}

#undef HudNumberTargets
