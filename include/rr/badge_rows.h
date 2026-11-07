#ifndef RR_BADGE_ROWS_H
#define RR_BADGE_ROWS_H

#include "rr/ui_bar.h"
#include "rr/icon_sprites.h"

#include "rr/badge_row_types.h"

extern RrBadgeRow1C *BadgeRows;
extern s8 BadgePatternTable[];
extern RrUiUvRect BadgeUvTable[];

/* The helpers thread full packet pointers. Coordinate arguments remain wide
 * until their wire16 stores, including sums of two signed16 row fields. */
void *AddTilePrim(u32 *, void *, s32, s32, s32, s32, s32, s32, s32);
void *AddTileGrid2x2(u32 *, void *, s32, s32, s32, s32, s32, s32, s32);
void *AddSpriteWithDrawMode(u32 *, void *, s32, s32, s32, s32, s32, s32,
                    u16, u16, u16, u16);
void *AddDrawAreaPrim(u32 *, void *, s32, s32, s32, s32);

/* Emits five row underlines/areas and four textured badges. Style selects an
 * eight-byte pattern group, with nine readable bytes; its first four signed
 * glyph indices must name accessible UV rows. Index selects five consecutive
 * records and the byte offsets must fit signed32. Output helpers determine each
 * next cursor. Captures glyph/shift before the first helper, then reloads the
 * row/context pointers after every helper; UV bytes are read after the first.
 * Each helper input cursor and returned cursor must point into writable arenas. */
void *DrawBadgePatternRows(void *cursor, s32 style, s32 index);

RR_ASSERT_SIZE(RrBadgeRow1C, 0x1C);
RR_ASSERT_OFFSET(RrBadgeRow1C, status00, 0);
RR_ASSERT_OFFSET(RrBadgeRow1C, half02, 2);
RR_ASSERT_OFFSET(RrBadgeRow1C, x04, 4);
RR_ASSERT_OFFSET(RrBadgeRow1C, y06, 6);
RR_ASSERT_OFFSET(RrBadgeRow1C, width0C, 0xC);
RR_ASSERT_OFFSET(RrBadgeRow1C, x08, 8);
RR_ASSERT_OFFSET(RrBadgeRow1C, y0A, 0xA);
RR_ASSERT_OFFSET(RrBadgeRow1C, extent0E, 0xE);
RR_ASSERT_OFFSET(RrBadgeRow1C, state10, 0x10);
RR_ASSERT_OFFSET(RrBadgeRow1C, x14, 0x14);
RR_ASSERT_OFFSET(RrBadgeRow1C, y16, 0x16);
RR_ASSERT_OFFSET(RrBadgeRow1C, w18, 0x18);
RR_ASSERT_OFFSET(RrBadgeRow1C, h1A, 0x1A);

#endif
