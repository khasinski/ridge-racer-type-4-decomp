#ifndef RR_FADE_SPRITE_H
#define RR_FADE_SPRITE_H
#include "rr/ui_bar.h"
#include "rr/animated_sprite_grid.h"

/* Drawing-context view extending through the effect's ordering head. */
typedef struct {
    RrGpuRectangle viewport00;
    u8 pad08[0x90];
    u32 head98;
} RrFadeSpriteContext;
extern RrFadeSpriteContext *CurrentDrawBuf;
extern s32 FadeSpritePhase,FadeSpriteIntensityScale;
extern RrUiUvRect FadeSpriteRects[];
extern u32 FadeSpriteTpageModes[];
/* Index names an accessible UV entry. Pointer slots and returned packet must
 * be accessible and aligned; helper return must provide eight writable bytes.
 * Products wrap to signed32 before each division by256 (truncation toward0).
 * Only positive scale emits a sprite and a linked draw-mode command. */
void EmitFadeSprite(s32 index,s32 x,s32 y,s32 intensity);
RR_ASSERT_SIZE(RrFadeSpriteContext,0x9C);
RR_ASSERT_OFFSET(RrFadeSpriteContext,head98,0x98);
#endif
