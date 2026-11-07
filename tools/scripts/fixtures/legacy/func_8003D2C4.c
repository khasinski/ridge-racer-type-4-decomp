/* CC1_FLAGS: -fno-strength-reduce */
#include "rr/ui_bar.h"

/* Caller-facing O32 arguments retain full register words; the existing helpers
 * narrow coordinates and packet fields when writing the GPU representation. */
extern RrSpritePacket *AddSpritePrim(u32*,RrSpritePacket*,s32,s32,RrUiUvRect*,u16,s32);
extern RrSpritePacket *GameQueueSprite(u32*,RrSpritePacket*,s32,s32,s32,s32,s32,s32,s32);
extern void *AddTilePrim(u32*,void*,s32,s32,s32,s32,s32,s32,s32);

RrSpritePacket *DrawBadgeTitleBar(u32 *order,RrSpritePacket *cursor,s32 x,s32 y,s32 style) {
    /* ASM_BARRIER: keep the ordering pointer live across every helper call. */
    register u32 *ordering __asm__("$21")=order;
    s32 xpos,ypos,offset,index;
    /* ASM_BARRIER: retain the captured descriptor across live count reloads. */
    register RrUiBarDescriptor *descriptor __asm__("$18");
    /* ASM_BARRIER: stage each horizontal coordinate in the outgoing argument. */
    register s32 x_stage __asm__("$6")=x;
    /* ASM_BARRIER: stage each vertical coordinate in the outgoing argument. */
    register s32 y_stage __asm__("$7")=y;
    u8 *descriptor_base;
    /* ASM_BARRIER: form the descriptor byte offset in the stack-argument temporary. */
    register s32 selector __asm__("$2");
    s32 width,sampled;
    u32 *out_order;
    RrSpritePacket *out_cursor;
    /* ASM_BARRIER: capture the saved ordering pointer before position setup. */
    __asm__("" : "+r"(x_stage) : "r"(ordering));
    xpos=x_stage;
    x_stage=(s32)((u32)xpos+6U);
    ypos=y_stage;
    y_stage=(s32)((u32)ypos+5U);
    offset=4;
    index=0;
    descriptor_base=(u8*)UiBarDescriptors;
    selector=(u32)style*sizeof(RrUiBarDescriptor);
    descriptor=(RrUiBarDescriptor*)(descriptor_base+selector);
    cursor=AddSpritePrim(ordering,cursor,x_stage,y_stage,
                       &UiBarGlyphRects[descriptor->glyph],0x7FFB,0);
    out_order=ordering;
    out_cursor=cursor;
    x_stage=xpos;
    y_stage=ypos;
    /* ASM_BARRIER: consume the returned cursor before reusing v0 for width. */
    __asm__("" : "+r"(out_order), "+r"(out_cursor), "+r"(x_stage), "+r"(y_stage));
    width=offset;
    cursor=GameQueueSprite(out_order,out_cursor,x_stage,y_stage,width,24,20,72,0x7E3B);
    if(descriptor->count>0) {
        do {
            cursor=GameQueueSprite(ordering,cursor,(s32)((u32)xpos+(u32)offset),
                                ypos,8,24,24,72,0x7E3B);
            sampled=descriptor->count;
            /* ASM_BARRIER: sample the updated count before incrementing the index. */
            __asm__("" : "+r"(sampled), "+r"(index));
            index++;
            offset+=8;
        }while(index<sampled);
    }
    cursor=GameQueueSprite(ordering,cursor,(s32)((u32)xpos+(u32)offset),
                        ypos,8,24,32,72,0x7E3B);
    out_order=ordering;
    out_cursor=cursor;
    x_stage=(s32)((u32)xpos+4U);
    y_stage=(s32)((u32)ypos+24U);
    /* ASM_BARRIER: finish the final helper arguments before computing underline width. */
    __asm__("" : "+r"(out_order), "+r"(out_cursor), "+r"(x_stage), "+r"(y_stage) : : "$2");
    width=offset-4;
    return AddTilePrim(out_order,out_cursor,x_stage,y_stage,width,2,255,255,255);
}
