#include "rr/overlay662_palette_edit.h"

void garage_func_801243DC(void) {
    u16 val;

    if (garage_D_80127668.words[0] & 8) {
        if (garage_D_80127668.words[1] & 4) {
            garage_D_80127668.words[0] &= ~8;
            garage_func_8011E7C0(13, 63, 294, 346);
            return;
        }
    } else {
        if (garage_func_8011E6C8(149, 373, 65, 13) != 0) {
            return;
        }
        if (!(garage_D_80127668.words[1] & 1)) {
            return;
        }
        garage_D_80127668.words[4] = 49;
        garage_D_80127668.words[0] |= 8;
        garage_func_8011E7C0(149, 373, 64, 12);
        garage_D_80127788 = 0;
        garage_D_8012778C = (*(u16 *)((garage_D_801276A8.config22*2)+(RrPaletteAddress)(RrPalette16 *)garage_D_801276A8.palette48[garage_D_801276A8.active21]) >> 5) & 0x1F;
    }
    val = (garage_D_80127668.halves[10] - 149) / 2;
    if (garage_D_8012778C != val) {
        if (garage_D_80127788 == 0) {
            garage_D_80127788 = 1;
            garage_func_8011D120();
        }
    }
    *(u16 *)((garage_D_801276A8.config22*2)+(RrPaletteAddress)(RrPalette16 *)garage_D_801276A8.palette48[garage_D_801276A8.active21]) =
        (*(u16 *)((garage_D_801276A8.config22*2)+(RrPaletteAddress)(RrPalette16 *)garage_D_801276A8.palette48[garage_D_801276A8.active21]) & 0xFC1F) | (val << 5);
}
