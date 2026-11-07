#include "rr/garage_tool_action.h"

void garage_select_eraser_tool(s32 mode) {
    if (mode < 0) {
        garage_logo_state.optionIndices98.flat[1] = 1;
    } else if (mode > 0) {
        switch (mode) {
        case 1:

garage_logo_state.optionIndices98.flat[1] = 0;
        garage_logo_state.selectorsB8[3] = 5;
        garage_navigate_to_option(0);
            break;
        case 2:

garage_logo_state.optionIndices98.flat[2] = 0;
        garage_logo_state.selectorsB8[0] = 2;
        garage_logo_state.selectorsB8[3] = 7;
        garage_navigate_to_option(0);
            break;
        case 4:
        garage_logo_state.selectorsB8[1] = 1;
        garage_logo_state.selectorsB8[2] = 0;
        garage_navigate_to_option(1);
            break;
        case 8:
        garage_logo_state.selectorsB8[1] = 3;
        garage_logo_state.selectorsB8[2] = 2;
        garage_navigate_to_option(1);
            break;
        }
    } else {
        if (garage_cursor_outside_button(0xE) == 0) {
            if (garage_logo_work.words[1] & 1) {
                if (garage_logo_work.words[0] & 4) {
                    garage_save_cursor_selection();
                } else {
                    garage_logo_state.optionIndices98.flat[1] = 1;
                    garage_logo_state.optionIndices98.flat[5] = 0;
                    garage_logo_state.saved64[0] = 0xE;
                    garage_logo_state.saved64[1] = 1;
                    garage_logo_state.saved64[2] = 2;
                    garage_logo_state.saved64[3] = 1;
                    garage_logo_state.saved64[4] = 6;
                }
                garage_logo_state.config1E = 3;
                PlaySfx(1);
            }
        }
    }
}
