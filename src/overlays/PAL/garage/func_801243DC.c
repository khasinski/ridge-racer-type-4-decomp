#include "rr/overlay662_palette_edit.h"

void garage_palette_green_slider(void) {
    u16 val;

    if (garage_logo_work.words[0] & 8) {
        if (garage_logo_work.words[1] & 4) {
            garage_logo_work.words[0] &= ~8;
            garage_set_cursor_bounds(13, 63, 294, 346);
            return;
        }
    } else {
        if (garage_cursor_outside_rect(149, 373, 65, 13) != 0) {
            return;
        }
        if (!(garage_logo_work.words[1] & 1)) {
            return;
        }
        garage_logo_work.words[4] = 49;
        garage_logo_work.words[0] |= 8;
        garage_set_cursor_bounds(149, 373, 64, 12);
        garage_green_slider_backed_up = 0;
        garage_green_slider_start = (*(u16 *)((garage_logo_state.config22*2)+(RrPaletteAddress)(RrPalette16 *)garage_logo_state.palette48[garage_logo_state.active21]) >> 5) & 0x1F;
    }
    val = (garage_logo_work.halves[10] - 149) / 2;
    if (garage_green_slider_start != val) {
        if (garage_green_slider_backed_up == 0) {
            garage_green_slider_backed_up = 1;
            garage_save_undo_copy();
        }
    }
    *(u16 *)((garage_logo_state.config22*2)+(RrPaletteAddress)(RrPalette16 *)garage_logo_state.palette48[garage_logo_state.active21]) =
        (*(u16 *)((garage_logo_state.config22*2)+(RrPaletteAddress)(RrPalette16 *)garage_logo_state.palette48[garage_logo_state.active21]) & 0xFC1F) | (val << 5);
}
