#include "rr/garage_color_pick.h"


void garage_pick_color(void) {
    s32 x, y;
    u8 *img0, *img1;
    u8 c;

    if (garage_cursor_outside_rect(82, 71, 128, 256) != 0) {
        return;
    }
    if (!(garage_logo_work.words[1] & 2)) {
        return;
    }
    img0 = (u8 *)(RrGarageImage512 *)garage_logo_state.image38[garage_logo_state.active21];
    img1 = (u8 *)(RrGarageImage512 *)garage_logo_state.image40[garage_logo_state.active21];
    x = garage_logo_state.reset08[0] + (garage_logo_work.halves[10] - 82) / garage_logo_state.config1C;
    y = garage_logo_state.reset08[1] + ((garage_logo_work.halves[11] - 71) / garage_logo_state.config1C) / 2;
    c = (img0 + y * 32)[x / 2];
    if (x & 1) {
        c >>= 4;
    } else {
        c &= 0xF;
    }
    if (c != 0) {
        garage_logo_state.config22 = c;
        PlaySfx(1);
    } else {
        c = (img1 + y * 32)[x / 2];
        if (x & 1) {
            c >>= 4;
        } else {
            c &= 0xF;
        }
        if (c != 0) {
            garage_logo_state.config22 = c;
            PlaySfx(1);
        } else {
            PlaySfx(4);
        }
    }
}
