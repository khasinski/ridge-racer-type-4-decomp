#ifndef RR_OVERLAY662_SPRITES_H
#define RR_OVERLAY662_SPRITES_H
#include "rr/animated_sprite_grid.h"
/* Serialized twenty-byte sprite descriptors; key0C selects one of72 slots. */
typedef struct {
    s32 depth00;
    u16 width04,height06;
    u8 u08,v09;
    u16 clut0A;
    s32 key0C;
    u32 flags10;
} RrGarageSpriteRecord;
extern RrGarageSpriteRecord garage_sprite_records[72];
extern RrGarageSpriteRecord *garage_logo_sprites[72];
/* Negative index initializes the lookup and returnsNULL. Rendering requires an
 * initialized index0..71, a valid record, a sprite-array cursor and an ordering
 * head at scratch.order+128+depth00. RGB bytes are preserved. */
u8 *garage_add_ui_sprite(s32 index,u8 *cursor,s32 x,s32 y);
RR_ASSERT_SIZE(RrGarageSpriteRecord,20);
RR_ASSERT_OFFSET(RrGarageSpriteRecord,width04,4);
RR_ASSERT_OFFSET(RrGarageSpriteRecord,clut0A,10);
RR_ASSERT_OFFSET(RrGarageSpriteRecord,key0C,12);
RR_ASSERT_OFFSET(RrGarageSpriteRecord,flags10,16);
#endif
