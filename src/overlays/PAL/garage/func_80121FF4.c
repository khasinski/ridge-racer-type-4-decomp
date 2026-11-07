#include "rr/overlay662_tool_action.h"

void garage_func_80121FF4(s32 mode) {
    if (mode < 0) {
        garage_D_801276A8.optionIndices98.flat[2] = 0;
        garage_D_801276A8.optionIndices98.flat[5] = 2;
    } else if (mode > 0) {
        switch (mode) {
        case 1:

garage_D_801276A8.optionIndices98.flat[1] = 1;
        garage_D_801276A8.selectorsB8[0] = 1;
        garage_D_801276A8.selectorsB8[3] = 6;
        garage_func_8011E5C0(0);
            break;
        case 2:
        garage_D_801276A8.optionIndices98.flat[2] = 1;
        garage_D_801276A8.selectorsB8[3] = 8;
        garage_func_8011E5C0(0);
            break;
        case 4:
        garage_D_801276A8.optionIndices98.flat[5] = 1;
        garage_D_801276A8.selectorsB8[2] = 4;
        garage_func_8011E5C0(1);
            break;
        case 8:
        garage_D_801276A8.optionIndices98.flat[5] = 3;
        garage_D_801276A8.selectorsB8[2] = 6;
        garage_func_8011E5C0(1);
            break;
        }
    } else {
        if (garage_func_8011E734(0x1A) == 0) {
            if (garage_D_80127668.words[1] & 1) {
                if (garage_D_80127668.words[0] & 4) {
                    garage_func_8011E694();
                } else {
                    garage_D_801276A8.optionIndices98.flat[2] = 0;
                    garage_D_801276A8.optionIndices98.flat[5] = 2;
                    garage_D_801276A8.saved64[0] = 0x1A;
                    garage_D_801276A8.saved64[1] = 2;
                    garage_D_801276A8.saved64[2] = 2;
                    garage_D_801276A8.saved64[3] = 5;
                    garage_D_801276A8.saved64[4] = 7;
                }
                garage_D_801276A8.config22 = 6;
                PlaySfx(1);
            }
        }
    }
}
