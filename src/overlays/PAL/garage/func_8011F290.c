#include "rr/overlay662_tool_action.h"

void garage_func_8011F290(s32 mode) {
    s32 i, j;
    u32 *src, *dst;

    if (mode < 0) {
        garage_D_801276A8.optionIndices98.flat[0] = 1;
    } else if (mode > 0) {
        switch (mode) {
        case 1:
            garage_D_801276A8.optionIndices98.flat[0] = 0;
            garage_D_801276A8.selectorsB8[3] = 0;
            garage_func_8011E5C0(0);
            break;
        case 2:
            garage_D_801276A8.optionIndices98.flat[0] = 2;
            garage_D_801276A8.selectorsB8[3] = 2;
            garage_func_8011E5C0(0);
            break;
        case 4:
            garage_D_801276A8.optionIndices98.flat[3] = 1;
            garage_D_801276A8.selectorsB8[1] = 0;
            garage_D_801276A8.selectorsB8[2] = 1;
            garage_func_8011E5C0(1);
            break;
        case 8:
            garage_D_801276A8.selectorsB8[1] = 3;
            garage_D_801276A8.selectorsB8[2] = 3;
            garage_func_8011E5C0(1);
            break;
        }
    } else if (garage_func_8011E734(3) == 0 && (garage_D_80127668.words[1] & 1)) {
        if (garage_D_80127668.words[0] & 4) {
            garage_func_8011E694();
        } else {
            garage_D_801276A8.optionIndices98.flat[0] = 1;
            garage_D_801276A8.optionIndices98.flat[5] = 0;
            garage_D_801276A8.saved64[0] = 3;
            garage_D_801276A8.saved64[1] = 0;
            garage_D_801276A8.saved64[2] = 2;
            garage_D_801276A8.saved64[3] = 2;
            garage_D_801276A8.saved64[4] = 1;
        }
        garage_func_8011D120();
        ((RrGarageButton *)garage_D_801273B8[3])->timer0E = 7;
        PlaySfx(1);
        if (garage_D_801276A8.layer23 == 0) {
            src = garage_D_801276A8.image38[garage_D_801276A8.active21 == 0];
            dst = garage_D_801276A8.image38[garage_D_801276A8.active21];
        } else {
            src = garage_D_801276A8.image40[garage_D_801276A8.active21 == 0];
            dst = garage_D_801276A8.image40[garage_D_801276A8.active21];
        }
        for (i = 0; i < 64; i++) {
            u32 *row;
            j = 0;
            row = (u32 *)(((63 - i) << 5) + (RrGarageImageAddress)src);
            do {
                *dst++ = *row++;
                j++;
            } while (j < 8);
        }
    }
}
