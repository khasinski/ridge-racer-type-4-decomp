#ifndef RR_SPRITE_PRESET_H
#define RR_SPRITE_PRESET_H
#include "rr/animated_sprite_grid.h"

/* Packed position, size and UV/CLUT words consumed by the sprite command. */
typedef struct {u32 position,size,uv;} RrSpritePreset;
extern RrSpritePreset SpritePresets[];

/* Zero intensity does nothing. Otherwise clamps intensity to 0..255 and emits
 * one sprite, even when a negative intensity clamps to zero. Index must name
 * an accessible nonnegative table entry; cursor must have 20 writable bytes.
 * Scratch order points at a page whose second word is the ordering head.
 * The page pointer is reloaded after each insertion store. */
void EmitPresetSprite(s32 intensity,s32 index);
RR_ASSERT_SIZE(RrSpritePreset, 12);
RR_ASSERT_OFFSET(RrSpritePreset, size, 4);
RR_ASSERT_OFFSET(RrSpritePreset, uv, 8);
#endif
