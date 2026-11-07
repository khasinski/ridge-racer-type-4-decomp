#include "rr/overlay662_tool_action.h"

void garage_func_8011F678(s32 mode) {
    u32 *dst;
    s32 i;

    if (mode < 0) {
        garage_D_801276A8.optionIndices98.flat[0] = 2;
        garage_D_801276A8.optionIndices98.flat[3] = 1;
    } else if (mode > 0) {
        switch (mode) {
        case 1:
            garage_D_801276A8.optionIndices98.flat[0] = 1;
            garage_D_801276A8.selectorsB8[3] = 1;
            garage_func_8011E5C0(0);
            break;
        case 2:
            garage_D_801276A8.optionIndices98.flat[0] = 3;
            garage_D_801276A8.selectorsB8[3] = 3;
            garage_func_8011E5C0(0);
            break;
        case 4:
            garage_D_801276A8.optionIndices98.flat[3] = 0;
            garage_D_801276A8.selectorsB8[2] = 0;
            garage_func_8011E5C0(1);
            break;
        case 8:
            garage_D_801276A8.selectorsB8[1] = 2;
            garage_D_801276A8.selectorsB8[2] = 2;
            garage_func_8011E5C0(1);
            break;
        }
    } else if (garage_func_8011E734(5) == 0 && (garage_D_80127668.words[1] & 1)) {
        if (garage_func_8011D0C0() != 0) {
            ((RrGarageButton *)garage_D_801273B8[5])->timer0E = 7;
            PlaySfx(4);
        } else {
            if (garage_D_80127668.words[0] & 4) {
                garage_func_8011E694();
            } else {
                garage_D_801276A8.optionIndices98.flat[0] = 2;
                garage_D_801276A8.optionIndices98.flat[3] = 1;
                garage_D_801276A8.saved64[0] = 5;
                garage_D_801276A8.saved64[1] = 0;
                garage_D_801276A8.saved64[2] = 0;
                garage_D_801276A8.saved64[3] = 1;
                garage_D_801276A8.saved64[4] = 2;
            }
            garage_func_8011D120();
            ((RrGarageButton *)garage_D_801273B8[5])->timer0E = 7;
            PlaySfx(1);
            if (garage_D_801276A8.layer23 == 0) {
                dst = garage_D_801276A8.image38[garage_D_801276A8.active21];
            } else {
                dst = garage_D_801276A8.image40[garage_D_801276A8.active21];
            }
            for (i = 0; i < 512; i++) {
                *dst++ = 0;
            }
        }
    }
}
