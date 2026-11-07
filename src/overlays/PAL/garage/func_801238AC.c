#include "rr/overlay662_palette_edit.h"
#include "rr/overlay662_tool_action.h"

void garage_func_801238AC(void) {
    u16 *pal = (u16 *)((RrPaletteAddress)(RrPalette16 *)garage_D_801276A8.palette48[garage_D_801276A8.active21]+(garage_D_801276A8.config22*2));
    u16 val = (*pal >> 5) & 0x1F;

    if (garage_D_80127668.words[0] & 8) {
        if (garage_D_80127668.words[1] & 4) {
            garage_D_80127668.words[0] &= ~8;
            garage_func_8011E7C0(13, 63, 294, 346);
            return;
        }
    } else {
        if (garage_func_8011E6C8(213, 373, 8, 12) != 0) {
            return;
        }
        if (!(garage_D_80127668.words[1] & 1)) {
            return;
        }
        garage_D_80127668.words[4] = 43;
        garage_D_80127668.words[0] |= 8;
        garage_func_8011E7DC(43);
        garage_func_8011D120();
    }
    if (garage_D_80127668.words[1] & 0x40) {
        val++;
        ((RrGarageButton *)garage_D_801273B8[43])->timer0E = 7;
        if (val >= 32) {
            val = 31;
            PlaySfx(4);
        } else {
            PlaySfx(0);
        }
        *pal = (*pal & 0xFC1F) | (val << 5);
    }
}
