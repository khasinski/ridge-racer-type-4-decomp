#include "rr/overlay662_tool_action.h"

void garage_select_layer2(s32 mode) {
    if (mode < 0) {

    } else if (mode > 0) {
        switch (mode) {
        case 1:
        garage_logo_state.selectorsB8[0] = 0;
        garage_logo_state.selectorsB8[3] = 0;
        garage_navigate_to_option(0);
            break;
        case 2:
        garage_logo_state.selectorsB8[0] = 2;
        garage_logo_state.selectorsB8[3] = 2;
        garage_navigate_to_option(0);
            break;
        case 4:
        garage_logo_state.selectorsB8[1] = 2;
        garage_logo_state.selectorsB8[2] = 1;
        garage_navigate_to_option(1);
            break;
        case 8:

garage_logo_state.selectorsB8[1] = 4;
        garage_logo_state.selectorsB8[2] = 3;
        garage_navigate_to_option(1);
            break;
        }
    } else {
        if (garage_cursor_outside_button(0x14) == 0) {
            if (garage_logo_work.words[1] & 1) {
                if (garage_logo_work.words[0] & 4) {
                    garage_save_cursor_selection();
                } else {
                    garage_logo_state.optionIndices98.flat[1] = 1;
                    garage_logo_state.optionIndices98.flat[6] = 0;
                    garage_logo_state.saved64[0] = 0x14;
                    garage_logo_state.saved64[1] = 1;
                    garage_logo_state.saved64[2] = 3;
                    garage_logo_state.saved64[3] = 2;
                    garage_logo_state.saved64[4] = 1;
                }
                ((RrGarageButton *)garage_logo_buttons[20])->timer0E = 7;
                garage_logo_state.layer23 = 1;
                PlaySfx(1);
            }
        }
    }
}
