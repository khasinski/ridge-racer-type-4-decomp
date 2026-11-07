#include "types.h"

/* Textured gouraud quad (GPU command 0x3E, 13-word packet). */
typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    u8 r1, g1, b1, pad1;
    s16 x1, y1;
    u8 u1, v1;
    u16 tpage;
    u8 r2, g2, b2, pad2;
    s16 x2, y2;
    u8 u2, v2;
    u16 pad3;
    u8 r3, g3, b3, pad4;
    s16 x3, y3;
    u8 u3, v3;
    u16 pad5;
} RrStrCreditQuad;

typedef struct {
    u8 pad00[0x70];
    u32 head70;
} RrStrCreditContext;

extern RrStrCreditContext *CurrentDrawBuf;
extern s32 D_80127E90;
const s32 movie_playback_D_80114898[2] = {0x35, 0x36};
const s32 movie_playback_D_801148A0 = 0x7F94;
s32 movie_playback_func_8011596C(s16 arg0);

#define RR_STR_SCRATCH_CURSOR (*(RrStrCreditQuad **)0x1F800000)

/* Draws one 8-pixel band of the scrolling credits texture. */
void movie_playback_func_801159C0(s32 line, s32 unused) {
    u8 color[4] = {0, 0, 255, 8};
    s32 page;
    u8 v;
    s32 y;
    s32 y2;
    s32 top;
    s32 bottom;
    RrStrCreditQuad *p;
    u32 *ot;

    page = line / 31;
    v = (line % 31) * 8;
    y = D_80127E90 / 100 + line * 8;
    color[1] = v;
    if ((u32)(y - 1) < 239) {
        top = movie_playback_func_8011596C(y) * 160 / 256;
        y2 = y + 8;
        bottom = movie_playback_func_8011596C(y2) * 160 / 256;
        p = RR_STR_SCRATCH_CURSOR;
        ((u8 *)p)[3] = 12;
        p->code = 0x3E;
        p->x0 = 32;
        p->y0 = y;
        p->x1 = 287;
        p->y1 = y;
        p->x2 = 32;
        p->y2 = y2;
        p->x3 = 287;
        p->y3 = y2;
        p->u0 = 0;
        p->v0 = v;
        p->u1 = 255;
        p->v1 = v;
        p->u2 = 0;
        p->v2 = v + 8;
        p->u3 = 255;
        p->v3 = v + 8;
        p->r0 = top;
        p->g0 = top;
        p->b0 = top;
        p->r1 = top;
        p->g1 = top;
        p->b1 = top;
        p->r2 = bottom;
        p->g2 = bottom;
        p->b2 = bottom;
        p->r3 = bottom;
        p->g3 = bottom;
        p->b3 = bottom;
        p->clut = movie_playback_D_801148A0;
        p->tpage = movie_playback_D_80114898[page];
        ot = &CurrentDrawBuf->head70;
        p->tag = (p->tag & 0xFF000000) | (*ot & 0x00FFFFFF);
        *ot = (*ot & 0xFF000000) | ((u32)p & 0x00FFFFFF);
        p++;
        RR_STR_SCRATCH_CURSOR = p;
    }
}
