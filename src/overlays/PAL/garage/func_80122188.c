#include "rr/overlay662_tool_action.h"

void garage_select_color7(s32 mode) {
    if (mode < 0) {
        garage_logo_state.optionIndices98.flat[2] = 0;
        garage_logo_state.optionIndices98.flat[5] = 3;
    } else if (mode > 0) {
        switch (mode) {
        case 1:
        garage_logo_state.optionIndices98.flat[1] = 1;
        garage_logo_state.selectorsB8[0] = 1;
        garage_logo_state.selectorsB8[3] = 6;
        garage_navigate_to_option(0);
            break;
        case 2:
        garage_logo_state.optionIndices98.flat[2] = 1;
        garage_logo_state.selectorsB8[3] = 8;
        garage_navigate_to_option(0);
            break;
        case 4:

        garage_logo_state.optionIndices98.flat[5] = 2;
        garage_logo_state.selectorsB8[2] = 5;
        garage_navigate_to_option(1);
            break;
        case 8:

        garage_logo_state.optionIndices98.flat[5] = 4;
        garage_logo_state.selectorsB8[2] = 7;
        garage_navigate_to_option(1);
            break;
        }
    } else {
        if (garage_cursor_outside_button(0x1B) == 0) {
            if (garage_logo_work.words[1] & 1) {
                if (garage_logo_work.words[0] & 4) {
                    garage_save_cursor_selection();
                } else {
                    garage_logo_state.optionIndices98.flat[2] = 0;
                    garage_logo_state.optionIndices98.flat[5] = 3;
                    garage_logo_state.saved64[0] = 0x1B;
                    garage_logo_state.saved64[1] = 2;
                    garage_logo_state.saved64[2] = 2;
                    garage_logo_state.saved64[3] = 6;
                    garage_logo_state.saved64[4] = 7;
                }
                garage_logo_state.config22 = 7;
                PlaySfx(1);
            }
        }
    }
}
