#include "rr/badge_rows.h"

void *func_80030BEC(void *cursor, s32 style, s32 index) {
    /* ASM_BARRIER: stage each returned cursor in A3 before outgoing arguments. */
    register void *result asm("$7") = cursor;
    /* ASM_BARRIER: preserve the unscaled style index in S7 across helpers. */
    register s32 selected asm("$23") = style;
    register s32 i = 0;
    const s8 *patterns = BadgePatternTable;
    register u8 *context_page = (u8 *)0x800B0000U;
    register u8 *row_page = (u8 *)0x800F0000U;
    s32 offset = index*28;
    register s32 xshift;
    register s32 glyph;
    const s8 *q;
    /* ASM_BARRIER: use V1 for row geometry until the final extent load. */
    register RrBadgeRow1C *p asm("$3");
    register RrBadgeRow1C *last;
    /* ASM_BARRIER: capture the helper cursor in A1 before preparing coordinates. */
    register void *outgoing asm("$5");
    /* ASM_BARRIER: stage the atlas base in V0 before scaling the glyph. */
    register RrUiUvRect *atlas asm("$2");
    register RrUiUvRect *uv;
    do {
        q = (const s8 *)((unsigned long)(i+selected*8)+(unsigned long)patterns);
        xshift = q[4];
        glyph = q[0];
        p = (RrBadgeRow1C *)((unsigned long)offset+(unsigned long)(*(RrBadgeRow1C **)(row_page+0x3AFC)));
        result = AddTilePrim(&(*(RrIconSpriteContext **)(context_page-0x33EC))->head70,result,p->x08,(s32)p->y0A+p->extent0E-2,248,2,8,8,8);
        if(i<4) {
            outgoing = result;
            atlas = BadgeUvTable;
            uv = atlas+glyph;
            p = (RrBadgeRow1C *)((unsigned long)offset+(unsigned long)(*(RrBadgeRow1C **)(row_page+0x3AFC)));
            result = AddSpriteWithDrawMode(&(*(RrIconSpriteContext **)(context_page-0x33EC))->head70,outgoing,(s32)p->x08+xshift,(s32)p->y06-2,uv->w,uv->h,uv->u,uv->v,74,0x2E28,1,128);
        }
        outgoing = result;
        i++;
        last = (RrBadgeRow1C *)((unsigned long)offset+(unsigned long)(*(RrBadgeRow1C **)(row_page+0x3AFC)));
        result = AddDrawAreaPrim(&(*(RrIconSpriteContext **)(context_page-0x33EC))->head70,outgoing,last->x14,last->y16,last->w18,last->h1A);
        offset+=28;
        /* ASM_BARRIER: end the returned V0 lifetime before the loop comparison. */
        __asm__("" : "+r"(result));
    }while(i<5);
    return result;
}
