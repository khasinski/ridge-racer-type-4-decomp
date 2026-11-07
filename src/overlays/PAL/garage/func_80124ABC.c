#include "rr/overlay662_image_stroke.h"

void garage_func_80124ABC(void) {
    u32 *img;

    if (garage_D_80127668.words[0] & 8) {
        if (garage_D_80127668.words[1] & 4) {
            garage_D_801276A8.config20 = 0;
            garage_D_80127668.words[0] &= ~8;
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
        garage_D_80127668.words[0] |= 8;
        garage_func_8011E7C0(82, 71, 128, 256);
        garage_func_8011D120();
        garage_D_80127798.x = garage_D_801276A8.reset08[0] + garage_D_801276A8.reset08[2];
        garage_D_80127798.y = garage_D_801276A8.reset08[1] + garage_D_801276A8.reset08[3];
    }
    if (garage_D_801276A8.layer23 == 0) {
        img = garage_D_801276A8.image38[garage_D_801276A8.active21];
    } else {
        img = garage_D_801276A8.image40[garage_D_801276A8.active21];
    }
    garage_D_8012779C.x = garage_D_801276A8.reset08[0] + garage_D_801276A8.reset08[2];
    garage_D_8012779C.y = garage_D_801276A8.reset08[1] + garage_D_801276A8.reset08[3];
    garage_func_801259FC(img, garage_D_80127798.x, garage_D_80127798.y, garage_D_8012779C.x, garage_D_8012779C.y, garage_D_801276A8.config22, garage_D_801276A8.config1D);
    garage_D_80127798.x = garage_D_8012779C.x;
    garage_D_80127798.y = garage_D_8012779C.y;
}
