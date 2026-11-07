#include "rr/overlay662_tool_action.h"

void garage_select_move_tool(s32 mode) {
    if (mode < 0) {
        garage_logo_state.optionIndices98.flat[0] = 2;
        garage_logo_state.optionIndices98.flat[3] = 0;
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
            garage_logo_state.selectorsB8[1] = 4;
            garage_logo_state.selectorsB8[2] = 4;
            garage_navigate_to_option(1);
            break;
        case 8:
            garage_logo_state.optionIndices98.flat[3] = 1;
            garage_logo_state.selectorsB8[2] = 1;
            garage_navigate_to_option(1);
            break;
        }
    } else if (garage_cursor_outside_button(4) == 0 && (garage_logo_work.words[1] & 1)) {
        if (garage_logo_work.words[0] & 4) {
            garage_save_cursor_selection();
        } else {
            garage_logo_state.optionIndices98.flat[0] = 2;
            garage_logo_state.optionIndices98.flat[3] = 0;
            garage_logo_state.saved64[0] = 4;
            garage_logo_state.saved64[1] = 0;
            garage_logo_state.saved64[2] = 0;
            garage_logo_state.saved64[3] = 0;
            garage_logo_state.saved64[4] = 2;
        }
        garage_logo_state.config1E = 4;
        PlaySfx(3);
        garage_logo_work.words[0] |= 2;
        garage_logo_work.points.first20 = garage_logo_work.points.cursor14;
        garage_logo_work.points.second24.x = 146;
        garage_logo_work.points.second24.y = 199;
        garage_logo_work.points.word28 = 0;
        garage_logo_work.points.distance2C = garage_calc_glide_step(&garage_logo_work.points.first20, &garage_logo_work.points.second24);
    }
}
