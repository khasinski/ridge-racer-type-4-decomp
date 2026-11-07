#include "rr/overlay662_tool_action.h"

void garage_select_layer1(s32 mode) {
    if (mode < 0) {

    } else if (mode > 0) {
        switch (mode) {
        case 1:
        garage_logo_state.selectorsB8[0] = 2;
        garage_logo_state.selectorsB8[3] = 2;
        garage_navigate_to_option(0);
            break;
        case 2:
        garage_logo_state.selectorsB8[0] = 1;
        garage_logo_state.selectorsB8[3] = 1;
        garage_navigate_to_option(0);
            break;
        case 4:
        garage_logo_state.selectorsB8[1] = 2;
        garage_logo_state.selectorsB8[2] = 2;
        garage_navigate_to_option(1);
            break;
        case 8:
        garage_logo_state.selectorsB8[1] = 4;
        garage_logo_state.selectorsB8[2] = 4;
        garage_navigate_to_option(1);
            break;
        }
    } else {
        if (garage_cursor_outside_button(0x13) == 0) {
            if (garage_logo_work.words[1] & 1) {
                if (garage_logo_work.words[0] & 4) {
                    garage_save_cursor_selection();
                } else {
                    garage_logo_state.optionIndices98.flat[0] = 4;
                    garage_logo_state.optionIndices98.flat[6] = 0;
                    garage_logo_state.saved64[0] = 0x13;
                    garage_logo_state.saved64[1] = 0;
                    garage_logo_state.saved64[2] = 3;
                    garage_logo_state.saved64[3] = 3;
                    garage_logo_state.saved64[4] = 0;
                }
                ((RrGarageButton *)garage_logo_buttons[19])->timer0E = 7;
                garage_logo_state.layer23 = 0;
                PlaySfx(1);
            }
        }
    }
}
