#ifndef RR_GARAGE_PALETTE_TILE_H
#define RR_GARAGE_PALETTE_TILE_H
#include "rr/garage_primitives.h"
#include "rr/palette16.h"
/* Palette slots0..15 correspond to initialized rectangle-table entries20..35.
 * active21 selects one of two complete16-entry palettes. Emits a TILE, then
 * optional sprite69 at signed low16(x-1,y-2) for the freshly selected slot.
 * Scratch cursor backing covers a tile and its optional following sprite. */
void garage_draw_palette_swatch(s32 index);
#endif
