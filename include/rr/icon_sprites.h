#ifndef RR_ICON_SPRITES_H
#define RR_ICON_SPRITES_H
#include "rr/animated_sprite_grid.h"

/* Prefix of the drawing context, including its first ordering head. */
typedef struct {
    RrGpuRectangle viewport00;
    u8 pad08[0x68];
    u32 head70;
} RrIconSpriteContext;
extern RrIconSpriteContext *CurrentDrawBuf;
extern u16 HudMapIconCluts[];

/* Requires a valid nonnegative CLUT-table index, a four-byte-aligned cursor
 * with 28 writable bytes, and accessible context heads on both captures.
 * Emits a 24x16 sprite at x148,y120/128, then draw-mode command E100023E.
 * Color uses its low byte. Each insertion preserves the head's high byte. */
void EmitIconSpriteRow120(s32 index,s32 color);
void EmitIconSpriteRow128(s32 index,s32 color);
RR_ASSERT_SIZE(RrIconSpriteContext, 0x74);
RR_ASSERT_OFFSET(RrIconSpriteContext, head70, 0x70);
#endif
