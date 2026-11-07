#include "rr/garage_tool_action.h"

void garage_flip_tool_action(s32 mode) {
    s32 i, j;
    u32 *src, *dst;

    if (mode < 0) {
        garage_logo_state.optionIndices98.flat[0] = 1;
    } else if (mode > 0) {
        switch (mode) {
        case 1:
            garage_logo_state.optionIndices98.flat[0] = 0;
            garage_logo_state.selectorsB8[3] = 0;
            garage_navigate_to_option(0);
            break;
        case 2:
            garage_logo_state.optionIndices98.flat[0] = 2;
            garage_logo_state.selectorsB8[3] = 2;
            garage_navigate_to_option(0);
            break;
        case 4:
            garage_logo_state.optionIndices98.flat[3] = 1;
            garage_logo_state.selectorsB8[1] = 0;
            garage_logo_state.selectorsB8[2] = 1;
            garage_navigate_to_option(1);
            break;
        case 8:
            garage_logo_state.selectorsB8[1] = 3;
            garage_logo_state.selectorsB8[2] = 3;
            garage_navigate_to_option(1);
            break;
        }
    } else if (garage_cursor_outside_button(3) == 0 && (garage_logo_work.words[1] & 1)) {
        if (garage_logo_work.words[0] & 4) {
            garage_save_cursor_selection();
        } else {
            garage_logo_state.optionIndices98.flat[0] = 1;
            garage_logo_state.optionIndices98.flat[5] = 0;
            garage_logo_state.saved64[0] = 3;
            garage_logo_state.saved64[1] = 0;
            garage_logo_state.saved64[2] = 2;
            garage_logo_state.saved64[3] = 2;
            garage_logo_state.saved64[4] = 1;
        }
        garage_save_undo_copy();
        ((RrGarageButton *)garage_logo_buttons[3])->timer0E = 7;
        PlaySfx(1);
        if (garage_logo_state.layer23 == 0) {
            src = garage_logo_state.image38[garage_logo_state.active21 == 0];
            dst = garage_logo_state.image38[garage_logo_state.active21];
        } else {
            src = garage_logo_state.image40[garage_logo_state.active21 == 0];
            dst = garage_logo_state.image40[garage_logo_state.active21];
        }
        for (i = 0; i < 64; i++) {
            u32 *row;
            j = 0;
            row = (u32 *)(((63 - i) << 5) + (RrGarageImageAddress)src);
            do {
                *dst++ = *row++;
                j++;
            } while (j < 8);
        }
    }
}
