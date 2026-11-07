#include "rr/overlay662_history_labels.h"

void garage_func_80117DFC(u32 *arg0, void *arg1, s32 arg2) {
    s32 x;
    s32 y;
    s32 i = 0;
    x = BadgeRows[arg2].x04 + 8;
    y = BadgeRows[arg2].y06 + 32;
    for (i = 0; i < 4; i++) {
        if (garage_D_8012622C[i] == -1) {
            arg1 = AddTextSprites(arg0, arg1, x, y, FormatCountOfTotal(garage_D_801272BE, 24, 1), 0x7FF8, 0, 0);
            break;
        }
        arg1 = AddSpritePrim(arg0, arg1, x, y, &UiFontGlyphs[garage_D_8012622C[i]], 0x7FF8, 0);
        /* ASM_BARRIER: boost arg1 register allocation priority to $s0 */
        __asm__("" : "+r"(arg1));
        x += UiFontGlyphs[garage_D_8012622C[i]].w + 2;
    }
    AddDrawModePrim(arg0, arg1, 56);
}
