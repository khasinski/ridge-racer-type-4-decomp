#include "rr/overlay662_color_pick.h"


void garage_func_80125370(void) {
    s32 x, y;
    u8 *img0, *img1;
    u8 c;

    if (garage_func_8011E6C8(82, 71, 128, 256) != 0) {
        return;
    }
    if (!(garage_D_80127668.words[1] & 2)) {
        return;
    }
    img0 = (u8 *)(RrGarageImage512 *)garage_D_801276A8.image38[garage_D_801276A8.active21];
    img1 = (u8 *)(RrGarageImage512 *)garage_D_801276A8.image40[garage_D_801276A8.active21];
    x = garage_D_801276A8.reset08[0] + (garage_D_80127668.halves[10] - 82) / garage_D_801276A8.config1C;
    y = garage_D_801276A8.reset08[1] + ((garage_D_80127668.halves[11] - 71) / garage_D_801276A8.config1C) / 2;
    c = (img0 + y * 32)[x / 2];
    if (x & 1) {
        c >>= 4;
    } else {
        c &= 0xF;
    }
    if (c != 0) {
        garage_D_801276A8.config22 = c;
        PlaySfx(1);
    } else {
        c = (img1 + y * 32)[x / 2];
        if (x & 1) {
            c >>= 4;
        } else {
            c &= 0xF;
        }
        if (c != 0) {
            garage_D_801276A8.config22 = c;
            PlaySfx(1);
        } else {
            PlaySfx(4);
        }
    }
}
