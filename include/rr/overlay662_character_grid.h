#ifndef RR_OVERLAY662_CHARACTER_GRID_H
#define RR_OVERLAY662_CHARACTER_GRID_H
#include "rr/overlay662_history_labels.h"
extern u8 NameCharset[];
extern s8 garage_name_entry_button_glyphs[3];
extern s32 garage_name_key_index;
s32 rcos(s32 angle);
/* Row names a complete badge. Character bytes1..40 and the three nonnegative
 * UV keys must be readable. Drawing services return full-width arena pointers
 * and may change selection, tick, row root and glyph widths between calls. */
void garage_draw_name_char_grid(u32 *order,void *cursor,s32 row);
typedef unsigned long RrCharacterGridAddress;
typedef char RrCharacterGridAddressWidth[
    sizeof(RrCharacterGridAddress)==sizeof(void*) ? 1 : -1];
#endif
