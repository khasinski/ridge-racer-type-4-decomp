#include "rr/overlay662_tool_action.h"

void garage_func_8012184C(s32 mode) {
    if (mode < 0) {
        garage_D_801276A8.optionIndices98.flat[2] = 0;
        garage_D_801276A8.optionIndices98.flat[4] = 0;
    } else if (mode > 0) {
        switch (mode) {
        case 1:
        garage_D_801276A8.optionIndices98.flat[1] = 1;
        garage_D_801276A8.selectorsB8[0] = 1;
        garage_D_801276A8.selectorsB8[3] = 3;
        garage_func_8011E5C0(0);
            break;
        case 2:
        garage_D_801276A8.optionIndices98.flat[2] = 1;
        garage_D_801276A8.selectorsB8[3] = 5;
        garage_func_8011E5C0(0);
            break;
        case 4:
        garage_D_801276A8.selectorsB8[1] = 4;
        garage_D_801276A8.selectorsB8[2] = 9;
        garage_func_8011E5C0(1);
            break;
        case 8:

garage_D_801276A8.optionIndices98.flat[4] = 1;
        garage_D_801276A8.selectorsB8[2] = 1;
        garage_func_8011E5C0(1);
            break;
        }
    } else {
        if (garage_func_8011E734(0x15) == 0) {
            if (garage_D_80127668.words[1] & 1) {
                if (garage_D_80127668.words[0] & 4) {
                    garage_func_8011E694();
                } else {
                    garage_D_801276A8.optionIndices98.flat[2] = 0;
                    garage_D_801276A8.optionIndices98.flat[4] = 0;
                    garage_D_801276A8.saved64[0] = 0x15;
                    garage_D_801276A8.saved64[1] = 2;
                    garage_D_801276A8.saved64[2] = 1;
                    garage_D_801276A8.saved64[3] = 0;
                    garage_D_801276A8.saved64[4] = 4;
                }
                garage_D_801276A8.config22 = 1;
                PlaySfx(1);
            }
        }
    }
}
