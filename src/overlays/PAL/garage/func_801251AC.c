#include "rr/overlay662_image_drag.h"

void garage_move_layer_drag(void) {
    u32 *src;
    u32 *dst;

    if (garage_logo_work.words[0] & 8) {
        if (garage_logo_work.words[1] & 4) {
            garage_logo_state.config20 = 0;
            garage_logo_work.words[0] &= ~8;
            garage_logo_work.words[14] = 0;
            garage_set_cursor_bounds(13, 63, 294, 346);
            return;
        }
    } else {
        if (garage_cursor_outside_rect(82, 71, 128, 256) != 0) {
            return;
        }
        if (!(garage_logo_work.words[1] & 1)) {
            return;
        }
        garage_logo_state.config20 = 1;
        garage_logo_work.words[4] = 52;
        garage_logo_work.words[14] = 1;
        garage_logo_work.words[0] |= 8;
        garage_set_cursor_bounds(82, 71, 128, 256);
        garage_save_undo_copy();
        garage_shift_start_point.x = garage_logo_state.reset08[0] + garage_logo_state.reset08[6];
        garage_shift_start_point.y = garage_logo_state.reset08[1] + garage_logo_state.reset08[7];
    }
    if (garage_logo_state.layer23 == 0) {
        src = garage_logo_state.image38[!garage_logo_state.active21];
        dst = garage_logo_state.image38[garage_logo_state.active21];
    } else {
        src = garage_logo_state.image40[!garage_logo_state.active21];
        dst = garage_logo_state.image40[garage_logo_state.active21];
    }
    garage_shift_image_wrap(
        src,
        dst,
        garage_logo_state.reset08[0] + garage_logo_state.reset08[6] - garage_shift_start_point.x,
        garage_logo_state.reset08[1] + garage_logo_state.reset08[7] - garage_shift_start_point.y);
}
