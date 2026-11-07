#include "rr/overlay662_exit_dialog.h"

void garage_exit_dialog(s32 mode) {
    u8 capturedFlags;
    u8 phase;
    s32 selection;

    if (mode < 0) {
        return;
    }
    if (mode > 0) {
        switch (mode) {
    case 1:
        garage_logo_state.selectorsB8[0] = mode;
        garage_logo_state.selectorsB8[3] = mode;
        garage_navigate_to_option(0);
        return;
    case 2:
        garage_logo_state.selectorsB8[0] = 0;
        garage_logo_state.selectorsB8[3] = 0;
        garage_navigate_to_option(0);
        return;
    case 4:
        garage_logo_state.selectorsB8[1] = 3;
        garage_logo_state.selectorsB8[2] = 8;
        garage_navigate_to_option(1);
        return;
    case 8:
        garage_logo_state.optionIndices98.flat[4] = 0;
        garage_logo_state.selectorsB8[1] = 1;
        garage_logo_state.selectorsB8[2] = 0;
        garage_navigate_to_option(1);
        return;
        default:
            return;
        }
    }
        if (garage_logo_work.words[0] & 8) {
            capturedFlags = garage_logo_state.flagsD1;
            if (capturedFlags == 0) {
                if (garage_logo_state.fade00 == 0) {
                    garage_logo_work.words[0] = (s32) (garage_logo_work.words[0] & ~9);
                    garage_logo_state.fade00 = -1;
                    ModeGroup=1;ModeId=0x27;ModeSubState=0;return;
                }
            } else {
                phase = UiRows[0].byte14;
                switch ((s32) phase) {
                case 2:
                    if ((BadgeRows->state10 == 3) && (garage_logo_state.flagsD1 = (u8) (capturedFlags | 4), (DialogueState->st == 4))) {
                        if (PlayerControllers[0].type01 != 0x12) {
                            if (PlayerControllers[0].buttons0A & 0x1000) {
                                if (garage_logo_state.stateD2 != 0) {
                                    garage_logo_state.stateD2 = 0U;
                                    goto block_32;
                                }
                            } else if (PlayerControllers[0].buttons0A & 0x4000) {
                                if (garage_logo_state.stateD2 != 1) {
                                    garage_logo_state.stateD2 = 1U;
block_32:
                                    PlaySfx(0);
                                }
                            }
                            if (MenuConfirmPressed != 0) {
                                garage_logo_state.stateD2 = (u8) (garage_logo_state.stateD2 + 2);
                                PlaySfx(1);
                            }
                        }
                        selection = garage_logo_state.stateD2;
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
                            garage_logo_state.fade00 = 0x40;
                            return;
                        case 3:
                            UiRows[0].cells[0] = 0;
                            UiRows[1].cells[0] = 1;
                            UiRows[0].byte14 = selection;
                            UiRows[1].byte14 = selection;
                            BadgeRows->state10 = 4;
                            garage_logo_state.flagsD1 = (u8) (garage_logo_state.flagsD1 & 0xFB);
                            return;
                        }
                    }
                    break;
                case 5:
                case 6:
                    if (BadgeRows->state10 == 0xB) {
                        garage_logo_work.words[0] = (s32) (garage_logo_work.words[0] & ~9);
                        garage_logo_state.flagsD1 = 0U;
                        return;
                    }
                    break;
                case 9:
                    if (garage_logo_state.fade00 == 0) {
                        ModeGroup = 1;
                        ModeId = 0x27;
                        ModeSubState = 0;
                        return;
                    }
                    break;
                }
            }
        } else if ((garage_cursor_outside_button(0x12) == 0) && (garage_logo_work.words[1] & 1)) {
            if (garage_logo_work.words[0] & 4) {
                garage_save_cursor_selection();
            } else {
                garage_logo_state.saved64[0] = 0x12;
                garage_logo_state.optionIndices98.flat[2] = 0;
                garage_logo_state.optionIndices98.flat[7] = 0;
                garage_logo_state.saved64[1] = 2;
                garage_logo_state.saved64[2] = 4;
                garage_logo_state.saved64[3] = 9;
                garage_logo_state.saved64[4] = 2;
            }
            garage_logo_work.words[4] = 0x12;
            garage_logo_work.words[0] = (s32) (garage_logo_work.words[0] | 8);
            PlaySfx(1);
            garage_logo_work.words[0] = (s32) (garage_logo_work.words[0] | 1);
            if (garage_logo_state.dirtyD3 == 0) {
                garage_logo_state.fade00 = 0x40;
                return;
            }
            DialogueState->b4 = 7;
            DialogueState->b5 = -1;
            DialogueState->st = 0;
            garage_logo_state.flagsD1 = 3U;
            BadgeRows->half02 = 1;
            BadgeRows->state10 = 0;
            UiRows[0].byte14 = 0U;
            UiRows[1].byte14 = 0U;
            garage_logo_state.stateD2 = 1U;
        } else {
            return;
        }
}
