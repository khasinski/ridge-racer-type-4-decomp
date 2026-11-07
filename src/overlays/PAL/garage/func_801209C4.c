#include "rr/overlay662_tool_action.h"

void garage_merge_layers_action(s32 mode) {
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
        garage_logo_state.selectorsB8[1] = 3;
        garage_logo_state.selectorsB8[2] = 2;
        garage_navigate_to_option(1);
            break;
        case 8:
        garage_logo_state.selectorsB8[1] = 1;
        garage_logo_state.selectorsB8[2] = 0;
        garage_navigate_to_option(1);
            break;
        }
    } else {
        if (garage_cursor_outside_button(0x10) == 0) {
            if (garage_logo_work.words[1] & 1) {
                if (garage_logo_work.words[0] & 4) {
                    garage_save_cursor_selection();
                } else {
                    garage_logo_state.optionIndices98.flat[1] = 1;
                    garage_logo_state.optionIndices98.flat[7] = 0;
                    garage_logo_state.saved64[0] = 0x10;
                    garage_logo_state.saved64[1] = 1;
                    garage_logo_state.saved64[2] = 4;
                    garage_logo_state.saved64[3] = 3;
                    garage_logo_state.saved64[4] = 1;
                }
                garage_save_undo_copy();
                ((RrGarageButton *)garage_logo_buttons[16])->timer0E = 7;
                PlaySfx(1);
                garage_blend_image_nibbles(garage_logo_state.image38[garage_logo_state.active21], garage_logo_state.image40[garage_logo_state.active21]);
            }
        }
    }
}
