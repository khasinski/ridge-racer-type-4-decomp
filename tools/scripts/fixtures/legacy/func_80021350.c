/* CC1_FLAGS: -fno-strength-reduce */
#include "rr/hud_rotating_quad.h"
#include "rr/trig.h"
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
