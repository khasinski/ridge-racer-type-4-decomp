#include "rr/garage_pixel_dispatch.h"

void garage_canvas_input(s32 actionCode)
{
    s32 i;
    u32 *clear;

    garage_map_cursor_to_cell();
    clear = garage_logo_state.scratch34;
    for (i = 511; i >= 0; i--) {
        *clear++ = 0;
    }
    if (garage_logo_state.config20 != 2) {
        switch (garage_logo_state.config1E) {
        case 0: garage_pencil_stroke(); break;
        case 1: garage_line_stroke(); break;
        case 2: garage_fill_at_cursor(); break;
        case 3: garage_erase_stroke(); break;
        case 4: garage_move_layer_drag(); break;
        }
    }
    if (garage_logo_state.config20 != 1) {
        switch (garage_logo_state.config1F) {
        case 0: garage_pick_color(); break;
        case 1: garage_alt_erase_stroke(); break;
        case 2: garage_alt_undo_toggle(); break;
        }
    }
}
