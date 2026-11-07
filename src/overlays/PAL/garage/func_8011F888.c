#include "rr/overlay662_confirm.h"

void garage_func_8011F888(s32 mode) {
    if (mode < 0) {
        garage_D_801276A8.optionIndices98.flat[0] = 2;
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
    } else if (garage_D_80127668.words[0] & 8) {
        if (BadgeRows[0].state10 == 3) {
            if (garage_D_801276A8.fade00 < 0) {
                garage_D_801276A8.flagsD1 |= 4;
                if (DialogueState->st == 4 && PlayerControllers[0].type01 != 0x12 && PlayerControllers[0].a != 0) {
                    garage_D_801276A8.fade00 = 64;
                    PlaySfx(1);
                }
            } else if (garage_D_801276A8.fade00 == 0) {
                MenuResultCode = 6;
                ModeGroup = 1;
                ModeId = 71;
                ModeSubState = 0;
            }
        }
    } else if (garage_func_8011E734(6) == 0 && (garage_D_80127668.words[1] & 1)) {
        if (garage_D_80127668.words[0] & 4) {
            garage_func_8011E694();
        } else {
            garage_D_801276A8.optionIndices98.flat[0] = 2;
            garage_D_801276A8.optionIndices98.flat[5] = 0;
            garage_D_801276A8.saved64[0] = 6;
            garage_D_801276A8.saved64[1] = 0;
            garage_D_801276A8.saved64[2] = 2;
            garage_D_801276A8.saved64[3] = 2;
            garage_D_801276A8.saved64[4] = 2;
        }
        garage_D_80127668.words[4] = 6;
        garage_D_80127668.words[0] |= 8;
        PlaySfx(1);
        garage_D_80127668.words[0] |= 1;
        DialogueState->b4 = 4;
        DialogueState->b5 = -1;
        DialogueState->st = 0;
        garage_D_801276A8.flagsD1 = 1;
        BadgeRows[0].half02 = 1;
        BadgeRows[0].state10 = 0;
    }
}
