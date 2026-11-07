#include "rr/overlay662_palette_edit.h"

void garage_palette_red_slider(void) {
    u16 val;
    u16 tmp;

    if (garage_logo_work.words[0] & 8) {
        if (garage_logo_work.words[1] & 4) {
            garage_logo_work.words[0] &= ~8;
            garage_set_cursor_bounds(13, 63, 294, 346);
            return;
        }
    } else {
        if (garage_cursor_outside_rect(149, 355, 65, 13) != 0) {
            return;
        }
        if (!(garage_logo_work.words[1] & 1)) {
            return;
        }
        garage_logo_work.words[4] = 48;
        garage_logo_work.words[0] |= 8;
        garage_set_cursor_bounds(149, 355, 64, 12);
        garage_red_slider_backed_up = 0;
        garage_red_slider_start = *(u16 *)((garage_logo_state.config22*2)+(RrPaletteAddress)(RrPalette16 *)garage_logo_state.palette48[garage_logo_state.active21]) & 0x1F;
    }
    val = (garage_logo_work.halves[10] - 149) / 2;
    if (garage_red_slider_start != val) {
        if (garage_red_slider_backed_up == 0) {
            garage_red_slider_backed_up = 1;
            garage_save_undo_copy();
        }
    }
    tmp = *(u16 *)((garage_logo_state.config22*2)+(RrPaletteAddress)(RrPalette16 *)garage_logo_state.palette48[garage_logo_state.active21]) & 0xFFE0;
    *(u16 *)((garage_logo_state.config22*2)+(RrPaletteAddress)(RrPalette16 *)garage_logo_state.palette48[garage_logo_state.active21]) = tmp | val;
}
