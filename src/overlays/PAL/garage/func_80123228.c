#include "rr/garage_scroll_actions.h"

void garage_scroll_left_button(void) {
    if (garage_logo_work.words[0] & 8) {
        if (garage_logo_work.words[1] & 4) {
            garage_logo_work.words[0] &= ~8;
            garage_set_cursor_bounds(13, 63, 294, 346);
            return;
        }
    } else {
        if (garage_cursor_outside_rect(81, 335, 9, 12) != 0) {
            return;
        }
        if (!(garage_logo_work.words[1] & 1)) {
            return;
        }
        garage_logo_work.words[4] = 38;
        garage_logo_work.words[0] |= 8;
        garage_set_clip_from_button(38);
    }
    if (garage_logo_work.words[1] & 0x40) {
        ((RrGarageButton *)garage_logo_buttons[38])->timer0E = 7;
        if (--garage_logo_state.reset08[0] < 0) {
            garage_logo_state.reset08[0] = 0;
            PlaySfx(4);
        } else {
            PlaySfx(0);
        }
    }
}
