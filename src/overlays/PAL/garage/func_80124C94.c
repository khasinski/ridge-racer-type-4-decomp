#include "rr/overlay662_image_stroke.h"

void garage_line_stroke(void) {
    u32 *img;

    if (garage_logo_work.words[0] & 8) {
        if (garage_logo_work.words[1] & 4) {
            garage_logo_state.config20 = 0;
            garage_logo_work.words[0] &= ~8;
            garage_set_cursor_bounds(13, 63, 294, 346);
            if (garage_logo_state.layer23 == 0) {
                img = garage_logo_state.image38[garage_logo_state.active21];
            } else {
                img = garage_logo_state.image40[garage_logo_state.active21];
            }
        } else {
            img = garage_logo_state.scratch34;
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
        garage_logo_work.words[0] |= 8;
        garage_set_cursor_bounds(82, 71, 128, 256);
        garage_save_undo_copy();
        img = garage_logo_state.scratch34;
        garage_line_start_point.x = garage_logo_state.reset08[0] + garage_logo_state.reset08[2];
        garage_line_start_point.y = garage_logo_state.reset08[1] + garage_logo_state.reset08[3];
    }
    garage_draw_brush_line(
        img,
        garage_line_start_point.x,
        garage_line_start_point.y,
        garage_logo_state.reset08[0] + garage_logo_state.reset08[2],
        garage_logo_state.reset08[1] + garage_logo_state.reset08[3],
        garage_logo_state.config22,
        garage_logo_state.config1D);
}
