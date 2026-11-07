#include "rr/text_sprites.h"

void DrawFontString(s32 x, s32 y, const u8 *text, s32 color, s32 bank) {
    register RrSpritePacket *packet;
    register s32 current_x;
    register s32 glyph;
    s32 width;
    register const u8 *width_entry;
    RrIconSpriteContext *context;
    u32 *head;

    /* ASM_BARRIER: retain the biased packet-field cursor in T0. */
    register u8 *fields asm("$8");
    register s32 row;
    s32 v;
    /* ASM_BARRIER: retain Y in S1 while A1 becomes glyph and width address. */
    register s32 yy asm("$17");
    /* ASM_BARRIER: retain the ordering-head index in S0 across the loop. */
    register s32 selected asm("$16");
    /* ASM_BARRIER: retain glyph U in T2 until the coordinate stores. */
    register s32 u asm("$10");
    const u8 *map, *widths;
    s32 missing, command;
    u32 low, high;
    const u8 *lookup;
    packet = (RrSpritePacket *)((RrGpuScratchPointers *)0x1F800000U)->cursor;
    current_x = x;
    yy = y;
    selected = bank;
    if (*text != 0) {
        map = FontGlyphMapAsciiBase + 32;
        missing = 255;
        command = 0x66;
        widths = FontGlyphWidths;
        low = 0xFFFFFF;
        high = 0xFF000000;
        fields = (u8 *)packet + 14;
        do {
            lookup = (const u8 *)((unsigned long)*text + (unsigned long)map);
            glyph = lookup[-32];
            text++;
            if (glyph != missing) {
                fields[-11] = 4;
                fields[-7] = 0x64;
                row = (s32)glyph / 21;
                {
                    s32 remainder = (s32)glyph - row * 21;
                    /* ASM_BARRIER: finish the remainder-times-three value in V1. */
                    register s32 triple asm("$3") = remainder * 3;
                    /* ASM_BARRIER: expose the unscaled triple before the final shift. */
                    __asm__("" : : "r"(triple));
                    u = triple * 4;
                }
                v = row * 12 + 188;
                if (color == missing) {
                    fields[-7] = 0x65;
                } else {
                    fields[-7] = command;
                    fields[-10] = color;
                    fields[-9] = color;
                    fields[-8] = color;
                }
                width_entry = (const u8 *)((unsigned long)glyph + (unsigned long)widths);
                *(s16 *)(fields - 6) = current_x;
                *(s16 *)(fields - 4) = yy;
                width = *width_entry;
                *(s16 *)(fields + 4) = 12;
                *(s16 *)fields = 0x7F78;
                fields[-2] = u;
                fields[-1] = v;
                context = CurrentDrawBuf;
                head = (u32 *)((unsigned long)(selected * 4) + (unsigned long)context + 0x70);
                *(s16 *)(fields + 2) = width;
                fields += 20;
                *(u32 *)packet = (*(u32 *)packet & high) | (*head & low);
                *head = (*head & high) | ((u32)(unsigned long)packet & low);
                packet++;
                current_x = (s32)((u32)current_x + *width_entry);
            } else {
                current_x = (s32)((u32)current_x + 4U);
            }
        } while (*text != 0);
    }
    ((RrGpuScratchPointers *)0x1F800000U)->cursor = (u8 *)packet;
}
