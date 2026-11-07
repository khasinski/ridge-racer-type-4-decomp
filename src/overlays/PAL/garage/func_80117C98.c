#include "rr/overlay662_history_labels.h"

void garage_func_80117C98(u32 *arg0, void *arg1, s32 arg2) {
    char buf[8];
    s32 i;
    s32 x;
    s32 y;

    x = BadgeRows[arg2].x04 + 8;
    y = BadgeRows[arg2].y06 + 32;
    for (i = 0; i < 8; i++) {
        buf[i] = 0;
    }
    for (i = 0; i < 3; i++) {
        arg1 = AddSpritePrim(arg0, arg1, x, y, &UiFontGlyphs[garage_D_80126228[i]], 0x7FF8, 0);
        x += UiFontGlyphs[garage_D_80126228[i]].w + 2;
        if (i == 2) {
            FormatDecimalDigits(buf, garage_D_801272BC, 1);
            arg1 = AddTextSprites(arg0, arg1, x, y, buf, 0x7FF8, 0, 0);
        }
    }
    AddDrawModePrim(arg0, arg1, 56);
}
