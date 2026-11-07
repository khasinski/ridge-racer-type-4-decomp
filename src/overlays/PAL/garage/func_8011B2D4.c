#include "rr/garage_sprites.h"
#ifndef RR_GPU_SCRATCH
#define RR_GPU_SCRATCH ((RrGpuScratchPointers *)0x1F800000)
#endif
u8 *garage_add_ui_sprite(s32 index,u8 *cursor,s32 x,s32 y)
{
    u32 *ordering=RR_GPU_SCRATCH->order+128;
    RrSpritePacket *packet=(RrSpritePacket *)cursor;
    RrGarageSpriteRecord *record;
    s32 i;
    s32 code;
    if (index<0) {
        for (i=0;i<72;i++) {
            record=&garage_sprite_records[i];
            garage_logo_sprites[record->key0C]=record;
        }
        return 0;
    }
    record=garage_logo_sprites[index];
    packet->len=4;
    packet->code=0x64;
    code=record->flags10 &1;
    if (code) code=0x66;
    else code=0x64;
    packet->code=code;
    /* ASM_BARRIER: retain the original code-byte read after selecting the mode. */
    __asm__("" ::: "memory");
    packet->x=x;packet->y=y;
    packet->code |=1;
    packet->w=record->width04;
    packet->h=record->height06;
    packet->u=record->u08;packet->v=record->v09;
    packet->clut=record->clut0A;
    packet->addr=ordering[record->depth00];
    ordering[record->depth00]=(ordering[record->depth00]&0xFF000000) | ((unsigned long)packet&0xFFFFFF);
    return (u8 *)(packet+1);
}
