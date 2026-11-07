#include "rr/garage_scroll_actions.h"

void garage_scroll_down_button(void) {
    s32 limit = 64 - 64 / (garage_logo_zoom >> 1);

    if (garage_logo_work.words[0] & 8) {
        if (garage_logo_work.words[1] & 4) {
            garage_logo_work.words[0] &= ~8;
            garage_set_cursor_bounds(13, 63, 294, 346);
            return;
        }
    } else {
        if (garage_cursor_outside_rect(215, 311, 6, 18) != 0) {
            return;
        }
        if (!(garage_logo_work.words[1] & 1)) {
            return;
        }
        garage_logo_work.words[4] = 37;
        garage_logo_work.words[0] |= 8;
        garage_set_clip_from_button(37);
    }
    if (garage_logo_work.words[1] & 0x40) {
        ((RrGarageButton *)garage_logo_buttons[37])->timer0E = 7;
        if (++garage_logo_state.reset08[1] > limit) {
            garage_logo_state.reset08[1] = limit;
            PlaySfx(4);
        } else {
            PlaySfx(0);
        }
    }
}
