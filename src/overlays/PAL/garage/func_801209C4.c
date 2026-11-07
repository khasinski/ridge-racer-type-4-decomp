#include "rr/overlay662_tool_action.h"

void garage_func_801209C4(s32 mode) {
    if (mode < 0) {

    } else if (mode > 0) {
        switch (mode) {
        case 1:
        garage_D_801276A8.selectorsB8[0] = 0;
        garage_D_801276A8.selectorsB8[3] = 0;
        garage_func_8011E5C0(0);
            break;
        case 2:
        garage_D_801276A8.selectorsB8[0] = 2;
        garage_D_801276A8.selectorsB8[3] = 2;
        garage_func_8011E5C0(0);
            break;
        case 4:
        garage_D_801276A8.selectorsB8[1] = 3;
        garage_D_801276A8.selectorsB8[2] = 2;
        garage_func_8011E5C0(1);
            break;
        case 8:
        garage_D_801276A8.selectorsB8[1] = 1;
        garage_D_801276A8.selectorsB8[2] = 0;
        garage_func_8011E5C0(1);
            break;
        }
    } else {
        if (garage_func_8011E734(0x10) == 0) {
            if (garage_D_80127668.words[1] & 1) {
                if (garage_D_80127668.words[0] & 4) {
                    garage_func_8011E694();
                } else {
                    garage_D_801276A8.optionIndices98.flat[1] = 1;
                    garage_D_801276A8.optionIndices98.flat[7] = 0;
                    garage_D_801276A8.saved64[0] = 0x10;
                    garage_D_801276A8.saved64[1] = 1;
                    garage_D_801276A8.saved64[2] = 4;
                    garage_D_801276A8.saved64[3] = 3;
                    garage_D_801276A8.saved64[4] = 1;
                }
                garage_func_8011D120();
                ((RrGarageButton *)garage_D_801273B8[16])->timer0E = 7;
                PlaySfx(1);
                garage_func_80125F60(garage_D_801276A8.image38[garage_D_801276A8.active21], garage_D_801276A8.image40[garage_D_801276A8.active21]);
            }
        }
    }
}
