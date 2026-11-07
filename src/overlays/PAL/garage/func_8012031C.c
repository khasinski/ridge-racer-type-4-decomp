#include "rr/overlay662_tool_action.h"

void garage_func_8012031C(s32 mode) {
    if (mode < 0) {
        garage_D_801276A8.optionIndices98.flat[1] = 0;
    } else if (mode > 0) {
        switch (mode) {
        case 1:

garage_D_801276A8.optionIndices98.flat[0] = 4;
        garage_D_801276A8.selectorsB8[0] = 0;
        garage_D_801276A8.selectorsB8[3] = 4;
        garage_func_8011E5C0(0);
            break;
        case 2:
        garage_D_801276A8.optionIndices98.flat[1] = 1;
        garage_D_801276A8.selectorsB8[3] = 6;
        garage_func_8011E5C0(0);
            break;
        case 4:
        garage_D_801276A8.selectorsB8[1] = 1;
        garage_D_801276A8.selectorsB8[2] = 0;
        garage_func_8011E5C0(1);
            break;
        case 8:
        garage_D_801276A8.selectorsB8[1] = 3;
        garage_D_801276A8.selectorsB8[2] = 2;
        garage_func_8011E5C0(1);
            break;
        }
    } else {
        if (garage_func_8011E734(0xC) == 0) {
            if (garage_D_80127668.words[1] & 1) {
                if (garage_D_80127668.words[0] & 4) {
                    garage_func_8011E694();
                } else {
                    garage_D_801276A8.optionIndices98.flat[1] = 0;
                    garage_D_801276A8.optionIndices98.flat[5] = 0;
                    garage_D_801276A8.saved64[0] = 0xC;
                    garage_D_801276A8.saved64[1] = 1;
                    garage_D_801276A8.saved64[2] = 2;
                    garage_D_801276A8.saved64[3] = 1;
                    garage_D_801276A8.saved64[4] = 5;
                }
                garage_D_801276A8.config1E = 2;
                PlaySfx(1);
            }
        }
    }
}
