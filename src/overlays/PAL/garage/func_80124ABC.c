#include "rr/overlay662_image_stroke.h"

void garage_pencil_stroke(void) {
    u32 *img;

    if (garage_logo_work.words[0] & 8) {
        if (garage_logo_work.words[1] & 4) {
            garage_logo_state.config20 = 0;
            garage_logo_work.words[0] &= ~8;
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
        garage_logo_work.words[0] |= 8;
        garage_set_cursor_bounds(82, 71, 128, 256);
        garage_save_undo_copy();
        garage_pen_prev_point.x = garage_logo_state.reset08[0] + garage_logo_state.reset08[2];
        garage_pen_prev_point.y = garage_logo_state.reset08[1] + garage_logo_state.reset08[3];
    }
    if (garage_logo_state.layer23 == 0) {
        img = garage_logo_state.image38[garage_logo_state.active21];
    } else {
        img = garage_logo_state.image40[garage_logo_state.active21];
    }
    garage_pen_point.x = garage_logo_state.reset08[0] + garage_logo_state.reset08[2];
    garage_pen_point.y = garage_logo_state.reset08[1] + garage_logo_state.reset08[3];
    garage_draw_brush_line(img, garage_pen_prev_point.x, garage_pen_prev_point.y, garage_pen_point.x, garage_pen_point.y, garage_logo_state.config22, garage_logo_state.config1D);
    garage_pen_prev_point.x = garage_pen_point.x;
    garage_pen_prev_point.y = garage_pen_point.y;
}
