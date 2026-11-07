#include "rr/garage_tool_action.h"

void garage_clear_layer_action(s32 mode) {
    u32 *dst;
    s32 i;

    if (mode < 0) {
        garage_logo_state.optionIndices98.flat[0] = 2;
        garage_logo_state.optionIndices98.flat[3] = 1;
    } else if (mode > 0) {
        switch (mode) {
        case 1:
            garage_logo_state.optionIndices98.flat[0] = 1;
            garage_logo_state.selectorsB8[3] = 1;
            garage_navigate_to_option(0);
            break;
        case 2:
            garage_logo_state.optionIndices98.flat[0] = 3;
            garage_logo_state.selectorsB8[3] = 3;
            garage_navigate_to_option(0);
            break;
        case 4:
            garage_logo_state.optionIndices98.flat[3] = 0;
            garage_logo_state.selectorsB8[2] = 0;
            garage_navigate_to_option(1);
            break;
        case 8:
            garage_logo_state.selectorsB8[1] = 2;
            garage_logo_state.selectorsB8[2] = 2;
            garage_navigate_to_option(1);
            break;
        }
    } else if (garage_cursor_outside_button(5) == 0 && (garage_logo_work.words[1] & 1)) {
        if (garage_is_layer_empty() != 0) {
            ((RrGarageButton *)garage_logo_buttons[5])->timer0E = 7;
            PlaySfx(4);
        } else {
            if (garage_logo_work.words[0] & 4) {
                garage_save_cursor_selection();
            } else {
                garage_logo_state.optionIndices98.flat[0] = 2;
                garage_logo_state.optionIndices98.flat[3] = 1;
                garage_logo_state.saved64[0] = 5;
                garage_logo_state.saved64[1] = 0;
                garage_logo_state.saved64[2] = 0;
                garage_logo_state.saved64[3] = 1;
                garage_logo_state.saved64[4] = 2;
            }
            garage_save_undo_copy();
            ((RrGarageButton *)garage_logo_buttons[5])->timer0E = 7;
            PlaySfx(1);
            if (garage_logo_state.layer23 == 0) {
                dst = garage_logo_state.image38[garage_logo_state.active21];
            } else {
                dst = garage_logo_state.image40[garage_logo_state.active21];
            }
            for (i = 0; i < 512; i++) {
                *dst++ = 0;
            }
        }
    }
}
