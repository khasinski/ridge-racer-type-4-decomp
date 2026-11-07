#include "rr/garage_scroll_actions.h"

void garage_scroll_up_button(void) {
    if (garage_logo_work.words[0] & 8) {
        if (garage_logo_work.words[1] & 4) {
            garage_logo_work.words[0] &= ~8;
            garage_set_cursor_bounds(13, 63, 294, 346);
            return;
        }
    } else {
        if (garage_cursor_outside_rect(215, 69, 6, 18) != 0) {
            return;
        }
        if (!(garage_logo_work.words[1] & 1)) {
            return;
        }
        garage_logo_work.words[4] = 36;
        garage_logo_work.words[0] |= 8;
        garage_set_clip_from_button(36);
    }
    if (garage_logo_work.words[1] & 0x40) {
        ((RrGarageButton *)garage_logo_buttons[36])->timer0E = 7;
        if (--garage_logo_state.reset08[1] < 0) {
            garage_logo_state.reset08[1] = 0;
            PlaySfx(4);
        } else {
            PlaySfx(0);
        }
    }
}
