#include "rr/overlay662_character_grid.h"

void garage_func_80117F54(u32 *order, void *cursor, s32 row)
{
    char text[4];
    s32 i, x, y, dx, dy, clut, color;
    register s32 v __asm__("$3");
    s32 shifted;
    RrCharacterGridAddress clearAddress;

    x = BadgeRows[row].x04 + 36;
    y = BadgeRows[row].y06 + 56;
    /* Convert only the four in-bounds addresses to pointers. The last integer
     * decrement never constructs a C pointer before the buffer. */
    for (i = 3, clearAddress = (RrCharacterGridAddress)(text + 3); i >= 0; i--) {
        *(char *)clearAddress = 0;
        --clearAddress;
    }
    for (i = 0; i < 40; i++) {
        text[0] = NameCharset[i + 1];
        dx = (i % 10) * 24;
        dy = (i / 10) * 32;
        if (i == garage_D_801273B4) {
            v = rcos((s32)(((u32)ScreenFrame << 7) - 682U));
            if (v >= 0) {
                clut = 32760;
                shifted = v >> 6;
                color = (shifted + 100) * 2;
            } else {
                clut = 32761;
                color = 0;
            }
        } else {
            clut = 32760;
            color = 0;
        }
        cursor = AddTextSprites(order, cursor, x + dx, y + dy, text, clut, 0, color);
    }
    cursor = AddTilePrim(order, cursor, x - 16, y + 136, 256, 2, 255, 255, 255);
    y += 144;
    x -= 12;
    for (i = 0, dx = i; i < 4; i++) {
        cursor = AddTilePrim(order, cursor, x + dx, y, 1, 16, 255, 255, 255);
        dx += 4;
        if (i < 3) {
            if (i == (s32)((u32)garage_D_801273B4 - 40U)) {
                v = rcos((s32)(((u32)ScreenFrame << 7) - 682U));
                if (v >= 0) {
                    clut = 32760;
                    shifted = v >> 6;
                    color = (shifted + 100) * 2;
                } else {
                    clut = 32761;
                    color = 0;
                }
            } else {
                clut = 32760;
                color = 0;
            }
            cursor = AddSpritePrim(order, cursor, x + dx, y,
                &UiFontGlyphs[garage_D_80126230[i]], clut, color);
            dx += UiFontGlyphs[garage_D_80126230[i]].w + 4;
        }
    }
    AddDrawModePrim(order, cursor, 56);
}
