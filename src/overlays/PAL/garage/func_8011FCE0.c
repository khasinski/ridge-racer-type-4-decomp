#include "rr/overlay662_tool_action.h"

void garage_cycle_zoom(s32 mode) {
    if (mode < 0) {
garage_logo_state.optionIndices98.flat[0] = 3;
    } else if (mode > 0) {
        switch (mode) {
        case 1:
        garage_logo_state.optionIndices98.flat[0] = 2;
        garage_logo_state.selectorsB8[3] = 2;
        garage_navigate_to_option(0);
            break;
        case 2:

        garage_logo_state.optionIndices98.flat[0] = 4;
        garage_logo_state.selectorsB8[3] = 4;
        garage_navigate_to_option(0);
            break;
        case 4:
        garage_logo_state.selectorsB8[1] = 0;
        garage_logo_state.selectorsB8[2] = 0;
        garage_navigate_to_option(1);
            break;
        case 8:
        garage_logo_state.selectorsB8[1] = 3;
        garage_logo_state.selectorsB8[2] = 3;
        garage_navigate_to_option(1);
            break;
        }
    } else {
        if (garage_cursor_outside_button(8) == 0) {
            if (garage_logo_work.words[1] & 1) {
                if (garage_logo_work.words[0] & 4) {
                    garage_save_cursor_selection();
                } else {
                    garage_logo_state.optionIndices98.flat[0] = 3;
                    garage_logo_state.optionIndices98.flat[5] = 0;
                    garage_logo_state.saved64[0] = 8;
                    garage_logo_state.saved64[1] = 0;
                    garage_logo_state.saved64[2] = 2;
                    garage_logo_state.saved64[3] = 2;
                    garage_logo_state.saved64[4] = 3;
                }
                ((RrGarageButton *)garage_logo_buttons[8])->timer0E = 7;
                PlaySfx(1);
                garage_logo_state.config1C <<= 1;
                if (garage_logo_state.config1C >= 0x11) {
                    garage_logo_state.config1C = 2U;
                    garage_logo_state.reset08[1] = 0;
                    garage_logo_state.reset08[0] = 0;
                }
            }
        }
    }
}
