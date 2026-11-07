#include "rr/overlay662_tool_action.h"

void garage_func_8011F4C0(s32 mode) {
    if (mode < 0) {
        garage_D_801276A8.optionIndices98.flat[0] = 2;
        garage_D_801276A8.optionIndices98.flat[3] = 0;
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
            garage_D_801276A8.selectorsB8[1] = 4;
            garage_D_801276A8.selectorsB8[2] = 4;
            garage_func_8011E5C0(1);
            break;
        case 8:
            garage_D_801276A8.optionIndices98.flat[3] = 1;
            garage_D_801276A8.selectorsB8[2] = 1;
            garage_func_8011E5C0(1);
            break;
        }
    } else if (garage_func_8011E734(4) == 0 && (garage_D_80127668.words[1] & 1)) {
        if (garage_D_80127668.words[0] & 4) {
            garage_func_8011E694();
        } else {
            garage_D_801276A8.optionIndices98.flat[0] = 2;
            garage_D_801276A8.optionIndices98.flat[3] = 0;
            garage_D_801276A8.saved64[0] = 4;
            garage_D_801276A8.saved64[1] = 0;
            garage_D_801276A8.saved64[2] = 0;
            garage_D_801276A8.saved64[3] = 0;
            garage_D_801276A8.saved64[4] = 2;
        }
        garage_D_801276A8.config1E = 4;
        PlaySfx(3);
        garage_D_80127668.words[0] |= 2;
        garage_D_80127668.points.first20 = garage_D_80127668.points.cursor14;
        garage_D_80127668.points.second24.x = 146;
        garage_D_80127668.points.second24.y = 199;
        garage_D_80127668.points.word28 = 0;
        garage_D_80127668.points.distance2C = garage_func_8011DBD8(&garage_D_80127668.points.first20, &garage_D_80127668.points.second24);
    }
}
