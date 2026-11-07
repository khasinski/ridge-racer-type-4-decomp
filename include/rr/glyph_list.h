#ifndef RR_GLYPH_LIST_H
#define RR_GLYPH_LIST_H
#include "rr/icon_sprites.h"
#include "rr/ui_bar.h"
typedef struct {u32 addr:24;u32 len:8;u32 command;} RrGlyphCommand8;
typedef struct {RrSpritePacketSignedClut sprite;RrGlyphCommand8 mode;} RrGlyphPacket28;
extern RrUiUvRect HudGlyphUvRectsBank0[50],HudGlyphUvRects[40];
#ifndef RR_GLYPH_SCRATCH
#define RR_GLYPH_SCRATCH ((RrGpuScratchPointers*)0x1F800000U)
#endif
void DrawGlyphList(s16*,s32,s32);
/* Readable signed16 x/y, then glyph/gap pairs and a negative terminator.
 * Bank0 selects primary glyph0..49; every nonzero bank selects0..39.
 * Glyph0x400..0x427 always selects the secondary40-entry font. Each glyph
 * needs28 writable aligned bytes in a live packet arena, disjoint from input
 * and font owners. Software cursors retain native width; only DMA wire tags
 * use low24 address bits. X arithmetic wraps32 before wire16 publication. */
RR_ASSERT_SIZE(RrGlyphCommand8,8);
RR_ASSERT_SIZE(RrGlyphPacket28,28);
RR_ASSERT_OFFSET(RrGlyphCommand8,command,4);
RR_ASSERT_OFFSET(RrGlyphPacket28,mode,20);
#endif
