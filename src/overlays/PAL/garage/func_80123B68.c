#include "rr/garage_palette_edit.h"
#include "rr/garage_tool_action.h"

void garage_palette_blue_up(void) {
    u16 *pal = (u16 *)((RrPaletteAddress)(RrPalette16 *)garage_logo_state.palette48[garage_logo_state.active21]+(garage_logo_state.config22*2));
    u16 val = (*pal >> 10) & 0x1F;

    if (garage_logo_work.words[0] & 8) {
        if (garage_logo_work.words[1] & 4) {
            garage_logo_work.words[0] &= ~8;
            garage_set_cursor_bounds(13, 63, 294, 346);
            return;
        }
    } else {
        if (garage_cursor_outside_rect(213, 391, 8, 12) != 0) {
            return;
        }
        if (!(garage_logo_work.words[1] & 1)) {
            return;
        }
        garage_logo_work.words[4] = 45;
        garage_logo_work.words[0] |= 8;
        garage_set_clip_from_button(45);
        garage_save_undo_copy();
    }
    if (garage_logo_work.words[1] & 0x40) {
        val++;
        ((RrGarageButton *)garage_logo_buttons[45])->timer0E = 7;
        if (val >= 32) {
            val = 31;
            PlaySfx(4);
        } else {
            PlaySfx(0);
        }
        *pal = (*pal & 0x83FF) | (val << 10);
    }
}
