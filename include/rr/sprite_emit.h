#ifndef RR_SPRITE_EMIT_H
#define RR_SPRITE_EMIT_H
#include "rr/ui_bar.h"
/* Resident emitters consume a full packet cursor and return the next cursor. */
void *AddSpritePrim(u32 *ordering,void *cursor,s32 x,s32 y,
    RrUiUvRect *region,u16 clut,s32 flags);
void *AddDrawModePrim(u32 *ordering,void *cursor,u16 page);
/* Coordinates remain wide until serialized to the packet's wire halfwords. */
RrSpritePacketSignedClut *GameQueueSprite(u32 *ordering,
    RrSpritePacketSignedClut *packet,s32 x,s32 y,s32 width,s32 height,
    s32 u,s32 v,s32 clut);
RrPolyF3Packet *EmitPolyF3(u32 *ordering,RrPolyF3Packet *packet,
    s32 xy0,s32 xy1,s32 xy2,const u8 *rgb,s32 flags);
#endif
