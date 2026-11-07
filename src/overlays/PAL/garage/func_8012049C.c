#include "rr/overlay662_tool_action.h"

void garage_func_8012049C(s32 mode) {
    if (mode < 0) {
        garage_D_801276A8.optionIndices98.flat[1] = 1;
    } else if (mode > 0) {
        switch (mode) {
        case 1:
        garage_D_801276A8.optionIndices98.flat[1] = 0;
        garage_D_801276A8.selectorsB8[3] = 2;
        garage_func_8011E5C0(0);
            break;
        case 2:
        garage_D_801276A8.optionIndices98.flat[2] = 0;
        garage_D_801276A8.selectorsB8[0] = 2;
        garage_D_801276A8.selectorsB8[3] = 4;
        garage_func_8011E5C0(0);
            break;
        case 4:
        garage_D_801276A8.selectorsB8[1] = 4;
        garage_D_801276A8.selectorsB8[2] = 3;
        garage_func_8011E5C0(1);
            break;
        case 8:

garage_D_801276A8.selectorsB8[1] = 2;
        garage_D_801276A8.selectorsB8[2] = 1;
        garage_func_8011E5C0(1);
            break;
        }
    } else {
        if (garage_func_8011E734(0xD) == 0) {
            if (garage_D_80127668.words[1] & 1) {
                if (garage_D_80127668.words[0] & 4) {
                    garage_func_8011E694();
                } else {
                    garage_D_801276A8.optionIndices98.flat[1] = 1;
                    garage_D_801276A8.optionIndices98.flat[4] = 1;
                    garage_D_801276A8.saved64[0] = 0xD;
                    garage_D_801276A8.saved64[1] = 1;
                    garage_D_801276A8.saved64[2] = 1;
                    garage_D_801276A8.saved64[3] = 0;
                    garage_D_801276A8.saved64[4] = 3;
                }
                ((RrGarageButton *)garage_D_801273B8[13])->timer0E = 7;
                garage_D_801276A8.config1D <<= 1;
                if (garage_D_801276A8.config1D >= 9) {
                    garage_D_801276A8.config1D = 1U;
                }
                PlaySfx(1);
            }
        }
    }
}
