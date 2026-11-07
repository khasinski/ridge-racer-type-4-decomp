#ifndef RR_TEXT_SPRITES_H
#define RR_TEXT_SPRITES_H

#include "rr/icon_sprites.h"

/* ROM byte view starts 32 bytes before the renderer's biased ASCII base.
 * The extra suffix keeps base+character inside its owner for every u8. */
extern u8 FontGlyphMapAsciiBase[288], FontGlyphWidths[256];

/* Text must terminate and each emitted glyph needs an aligned 20-byte sprite
 * packet in the scratch cursor's arena. The biased field cursor also requires
 * 14 bytes of accessible arena after the final packet. The context must contain
 * the selected aligned ordering head at 0x70+bank*4; bank*4 must fit signed32.
 * Mapping 255 advances X by four without emitting. Color exactly 255 selects
 * raw textured mode and preserves RGB; other values publish their low byte.
 * Widths and text are read again after packet stores/list insertion. Coordinates
 * and X increments wrap at 16/32 bits respectively. Height is 12 and width comes
 * from the first width-table read. */
void DrawFontString(s32 x, s32 y, const u8 *text, s32 color, s32 bank);

#endif
