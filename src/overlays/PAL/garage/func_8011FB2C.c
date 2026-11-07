#include "rr/overlay662_tool_action.h"

void garage_func_8011FB2C(s32 mode) {
    if (mode < 0) {
        garage_D_801276A8.optionIndices98.flat[0] = 3;
    } else if (mode > 0) {
        switch (mode) {
        case 1:
        garage_D_801276A8.optionIndices98.flat[0] = 2;
        garage_D_801276A8.selectorsB8[1] = 0;
        garage_D_801276A8.selectorsB8[3] = 2;
        garage_func_8011E5C0(0);
            break;
        case 2:
        garage_D_801276A8.optionIndices98.flat[0] = 4;
        garage_D_801276A8.selectorsB8[1] = 1;
        garage_D_801276A8.selectorsB8[3] = 1;
        garage_func_8011E5C0(0);
            break;
        case 4:
        garage_D_801276A8.selectorsB8[1] = 4;
        garage_D_801276A8.selectorsB8[2] = 4;
        garage_func_8011E5C0(1);
            break;
        case 8:
        garage_D_801276A8.selectorsB8[1] = 2;
        garage_D_801276A8.selectorsB8[2] = 2;
        garage_func_8011E5C0(1);
            break;
        }
    } else {
        if (garage_func_8011E734(7) == 0) {
            if (garage_D_80127668.words[1] & 1) {
                if (garage_D_80127668.words[0] & 4) {
                    garage_func_8011E694();
                } else {
                    garage_D_801276A8.optionIndices98.flat[0] = 3;
                    garage_D_801276A8.optionIndices98.flat[3] = 0;
                    garage_D_801276A8.saved64[0] = 7;
                    garage_D_801276A8.saved64[1] = 0;
                    garage_D_801276A8.saved64[2] = 0;
                    garage_D_801276A8.saved64[3] = 0;
                    garage_D_801276A8.saved64[4] = 3;
                }
                ((RrGarageButton *)garage_D_801273B8[7])->timer0E = 7;
                PlaySfx(1);
                garage_D_801276A8.config1F++;
                if (garage_D_801276A8.config1F >= 3) {
                    garage_D_801276A8.config1F = 0;
                }
            }
        }
    }
}
