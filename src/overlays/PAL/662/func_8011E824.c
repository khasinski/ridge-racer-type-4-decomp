#include "rr/overlay662_modal.h"

void ovl662_func_8011E824(s32 mode) {
    u8 phase;
    s32 selection;

    if (mode < 0) {
        ovl662_D_801276A8.optionIndices98.flat[0] = 0;
        return;
    }
    if (mode > 0) {
        switch (mode) {
    case 1:
        if (ovl662_D_801276A8.selectorsB8[1] == 0) {
            ovl662_D_801276A8.optionIndices98.flat[2] = mode;
            ovl662_D_801276A8.selectorsB8[0] = 2;
            ovl662_D_801276A8.selectorsB8[1] = mode;
            ovl662_D_801276A8.selectorsB8[3] = 5;
        } else {
            if (ovl662_D_801276A8.optionIndices98.flat[5] == 4) {
                ovl662_D_801276A8.optionIndices98.flat[2] = 0;
            } else {
                ovl662_D_801276A8.optionIndices98.flat[2] = mode;
            }
            ovl662_D_801276A8.selectorsB8[0] = 2;
            ovl662_D_801276A8.selectorsB8[1] = 2;
            ovl662_D_801276A8.selectorsB8[3] = 8;
        }
        ovl662_func_8011E5C0(0);
        return;
    case 2:
        ovl662_D_801276A8.optionIndices98.flat[0] = 1;
        ovl662_D_801276A8.selectorsB8[3] = 1;
        ovl662_func_8011E5C0(0);
        return;
    case 4:
        ovl662_D_801276A8.selectorsB8[1] = mode;
        ovl662_D_801276A8.selectorsB8[2] = mode;
        ovl662_func_8011E5C0(1);
        return;
    case 8:
        ovl662_D_801276A8.selectorsB8[1] = 3;
        ovl662_D_801276A8.selectorsB8[2] = 3;
        ovl662_func_8011E5C0(1);
        return;
        default:
            return;
        }
    }
        if (ovl662_D_80127668.words[0] & 8) {
            phase = UiRows[0].byte14;
            switch ((s32) phase) {
            case 2:
                if ((BadgeRows->state10 == 3) && (ovl662_D_801276A8.flagsD1 = (u8) (ovl662_D_801276A8.flagsD1 | 4), (DialogueState->st == 4))) {
                    if (PlayerControllers[0].type01 != 0x12) {
                        if (PlayerControllers[0].buttons0A & 0x1000) {
                            if (ovl662_D_801276A8.stateD2 != 0) {
                                ovl662_D_801276A8.stateD2 = 0U;
                                goto block_36;
                            }
                        } else if (PlayerControllers[0].buttons0A & 0x4000) {
                            if (ovl662_D_801276A8.stateD2 != 1) {
                                ovl662_D_801276A8.stateD2 = 1U;
block_36:
                                PlaySfx(0);
                            }
                        }
                        if (MenuConfirmPressed != 0) {
                            ovl662_D_801276A8.stateD2 = (u8) (ovl662_D_801276A8.stateD2 + 2);
                            PlaySfx(1);
                        }
                    }
                    selection = ovl662_D_801276A8.stateD2;
                    switch (selection) {
                    case 0:
                        UiRows[0].cells[0] = 2;
                        UiRows[1].cells[0] = 0;
                        return;
                    case 1:
                        UiRows[0].cells[0] = 0;
                        UiRows[1].cells[0] = 2;
                        return;
                    case 2:
                        UiRows[0].cells[0] = 1;
                        UiRows[1].cells[0] = 0;
                        UiRows[0].byte14 = 7U;
                        UiRows[1].byte14 = 7U;
                        ovl662_D_801276A8.fade00 = 0x40;
                        return;
                    case 3:
                        UiRows[0].cells[0] = 0;
                        UiRows[1].cells[0] = 1;
                        UiRows[0].byte14 = selection;
                        UiRows[1].byte14 = selection;
                        BadgeRows->state10 = 4;
                        ovl662_D_801276A8.flagsD1 = (u8) (ovl662_D_801276A8.flagsD1 & 0xFB);
                        return;
                    }
                }
                break;
            case 5:
            case 6:
                if (BadgeRows->state10 == 0xB) {
                    ovl662_D_80127668.words[0] = (s32) (ovl662_D_80127668.words[0] & ~9);
                    ovl662_D_801276A8.flagsD1 = 0;
                    return;
                }
                break;
            case 9:
                if (ovl662_D_801276A8.fade00 == 0) {
                    ovl662_D_801272E8 = 1;
                    ModeGroup = 1;
                    ModeId = 0x45;
                    D_800F4D8A = 0;
                    return;
                }
                break;
            }
        } else {
            if ((ovl662_func_8011E734(0) == 0) && (ovl662_D_80127668.words[1] & 1)) {
                if (ovl662_D_80127668.words[0] & 4) {
                    ovl662_func_8011E694();
                } else {
                    ovl662_D_801276A8.optionIndices98.flat[0] = 0;
                    ovl662_D_801276A8.optionIndices98.flat[3] = 0;
                    ovl662_D_801276A8.saved64[0] = 0;
                    ovl662_D_801276A8.saved64[1] = 0;
                    ovl662_D_801276A8.saved64[2] = 0;
                    ovl662_D_801276A8.saved64[3] = 0;
                    ovl662_D_801276A8.saved64[4] = 0;
                }
                ovl662_D_80127668.words[4] = 0;
                ovl662_D_80127668.words[0] = (s32) (ovl662_D_80127668.words[0] | 8);
                PlaySfx(1);
                ovl662_D_80127668.words[0] = (s32) (ovl662_D_80127668.words[0] | 1);
                DialogueState->b4 = 3;
                DialogueState->b5 = -1;
                DialogueState->st = 0;
                ovl662_D_801276A8.flagsD1 = 3U;
                BadgeRows->half02 = 1;
                BadgeRows->state10 = 0;
                UiRows[0].byte14 = 0U;
                UiRows[1].byte14 = 0U;
                ovl662_D_801276A8.stateD2 = 1U;
            }
            return;
        }
}
