#include "rr/garage_history_labels.h"

void garage_draw_maker_count_label(u32 *arg0, void *arg1, s32 arg2) {
    s32 i;
    s32 x;
    s32 y;
    s32 flag = 0;

    x = BadgeRows[arg2].x04 + 8;
    y = BadgeRows[arg2].y06 + 32;
    for (i = 0; i < 4; i++) {
        if (SelectionRows.bytes[0] == SaveProgress.history6C[i].selection &&
            SelectionRows.bytes[1] == SaveProgress.history6C[i].variant) {
            flag = rsin(((s32)(((u32)ScreenFrame << 7) - 682U)) & 0xFFE) < 0;
            break;
        }
    }
    for (i = 0; i < 3; i++) {
        if (garage_maker_cars_label_glyphs[i] == -1) {
            arg1 = AddTextSprites(
                arg0, arg1, x, y,
                FormatCountOfTotal(garage_team_maker_car_counts[SelectionRows.bytes[0]][SelectionRows.bytes[1]], 20, 1),
                flag + 0x7FF8, 0, 0);
            break;
        }
        arg1 = AddSpritePrim(arg0, arg1, x, y, &UiFontGlyphs[garage_maker_cars_label_glyphs[i]], 0x7FF8, 0);
        /* ASM_BARRIER: boost arg1 register allocation priority to $s2 */
        __asm__("" : "+r"(arg1));
        x += UiFontGlyphs[garage_maker_cars_label_glyphs[i]].w + 2;
    }
    AddDrawModePrim(arg0, arg1, 56);
}
