#include "rr/overlay662_image_drag.h"

void garage_func_801251AC(void) {
    u32 *src;
    u32 *dst;

    if (garage_D_80127668.words[0] & 8) {
        if (garage_D_80127668.words[1] & 4) {
            garage_D_801276A8.config20 = 0;
            garage_D_80127668.words[0] &= ~8;
            garage_D_80127668.words[14] = 0;
            garage_func_8011E7C0(13, 63, 294, 346);
            return;
        }
    } else {
        if (garage_func_8011E6C8(82, 71, 128, 256) != 0) {
            return;
        }
        if (!(garage_D_80127668.words[1] & 1)) {
            return;
        }
        garage_D_801276A8.config20 = 1;
        garage_D_80127668.words[4] = 52;
        garage_D_80127668.words[14] = 1;
        garage_D_80127668.words[0] |= 8;
        garage_func_8011E7C0(82, 71, 128, 256);
        garage_func_8011D120();
        garage_D_801277AC.x = garage_D_801276A8.reset08[0] + garage_D_801276A8.reset08[6];
        garage_D_801277AC.y = garage_D_801276A8.reset08[1] + garage_D_801276A8.reset08[7];
    }
    if (garage_D_801276A8.layer23 == 0) {
        src = garage_D_801276A8.image38[!garage_D_801276A8.active21];
        dst = garage_D_801276A8.image38[garage_D_801276A8.active21];
    } else {
        src = garage_D_801276A8.image40[!garage_D_801276A8.active21];
        dst = garage_D_801276A8.image40[garage_D_801276A8.active21];
    }
    garage_func_80125DE0(
        src,
        dst,
        garage_D_801276A8.reset08[0] + garage_D_801276A8.reset08[6] - garage_D_801277AC.x,
        garage_D_801276A8.reset08[1] + garage_D_801276A8.reset08[7] - garage_D_801277AC.y);
}
