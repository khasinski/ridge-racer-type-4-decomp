#include "rr/overlay662_palette_edit.h"

void garage_func_80124258(void) {
    u16 val;
    u16 tmp;

    if (garage_D_80127668.words[0] & 8) {
        if (garage_D_80127668.words[1] & 4) {
            garage_D_80127668.words[0] &= ~8;
            garage_func_8011E7C0(13, 63, 294, 346);
            return;
        }
    } else {
        if (garage_func_8011E6C8(149, 355, 65, 13) != 0) {
            return;
        }
        if (!(garage_D_80127668.words[1] & 1)) {
            return;
        }
        garage_D_80127668.words[4] = 48;
        garage_D_80127668.words[0] |= 8;
        garage_func_8011E7C0(149, 355, 64, 12);
        garage_D_80127780 = 0;
        garage_D_80127784 = *(u16 *)((garage_D_801276A8.config22*2)+(RrPaletteAddress)(RrPalette16 *)garage_D_801276A8.palette48[garage_D_801276A8.active21]) & 0x1F;
    }
    val = (garage_D_80127668.halves[10] - 149) / 2;
    if (garage_D_80127784 != val) {
        if (garage_D_80127780 == 0) {
            garage_D_80127780 = 1;
            garage_func_8011D120();
        }
    }
    tmp = *(u16 *)((garage_D_801276A8.config22*2)+(RrPaletteAddress)(RrPalette16 *)garage_D_801276A8.palette48[garage_D_801276A8.active21]) & 0xFFE0;
    *(u16 *)((garage_D_801276A8.config22*2)+(RrPaletteAddress)(RrPalette16 *)garage_D_801276A8.palette48[garage_D_801276A8.active21]) = tmp | val;
}
