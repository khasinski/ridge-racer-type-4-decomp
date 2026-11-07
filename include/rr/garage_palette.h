#ifndef RR_GARAGE_PALETTE_H
#define RR_GARAGE_PALETTE_H
#include "rr/garage_primitives.h"
/* The same32 palette bytes are copied as eight words and selected as16 colors. */
typedef union {u16 value;u8 bytes[2];} RrGaragePaletteEntry;
typedef union {
    u32 words[8];
    RrGaragePaletteEntry colors[16];
    u8 bytes[32];
} RrGaragePalette;
void garage_draw_color_panel(void);
/* Channel0..2; panel supplies a five-bit value. */
void garage_draw_color_channel(s32 channel,s32 value);
RR_ASSERT_SIZE(RrGaragePaletteEntry,2);
RR_ASSERT_SIZE(RrGaragePalette,32);
#endif
