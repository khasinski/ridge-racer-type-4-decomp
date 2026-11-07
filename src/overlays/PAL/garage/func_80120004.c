#include "rr/overlay662_tool_action.h"

void garage_func_80120004(s32 mode) {
    if (mode < 0) {
        garage_D_801276A8.optionIndices98.flat[0] = 4;
    } else if (mode > 0) {
        switch (mode) {
        case 1:

garage_D_801276A8.optionIndices98.flat[0] = 3;
        garage_D_801276A8.selectorsB8[3] = 3;
        garage_func_8011E5C0(0);
            break;
        case 2:
        garage_D_801276A8.selectorsB8[0] = 1;
garage_D_801276A8.optionIndices98.flat[1] = 0;
        garage_D_801276A8.selectorsB8[3] = 5;
        garage_func_8011E5C0(0);
            break;
        case 4:
        garage_D_801276A8.selectorsB8[1] = 1;
        garage_D_801276A8.selectorsB8[2] = 0;
        garage_func_8011E5C0(1);
            break;
        case 8:
        garage_D_801276A8.selectorsB8[1] = 3;
        garage_D_801276A8.selectorsB8[2] = 3;
        garage_func_8011E5C0(1);
            break;
        }
    } else {
        if (garage_func_8011E734(0xA) == 0) {
            if (garage_D_80127668.words[1] & 1) {
                if (garage_D_80127668.words[0] & 4) {
                    garage_func_8011E694();
                } else {
                    garage_D_801276A8.optionIndices98.flat[0] = 4;
                    garage_D_801276A8.optionIndices98.flat[5] = 0;
                    garage_D_801276A8.saved64[0] = 0xA;
                    garage_D_801276A8.saved64[1] = 0;
                    garage_D_801276A8.saved64[2] = 2;
                    garage_D_801276A8.saved64[3] = 2;
                    garage_D_801276A8.saved64[4] = 4;
                }
                ((RrGarageButton *)garage_D_801273B8[10])->timer0E = 7;
                garage_D_801276A8.active21 = (u8) ((u8) garage_D_801276A8.active21 < 1U);
                PlaySfx(1);
            }
        }
    }
}
