#include "rr/overlay662_tool_action.h"

void garage_swap_layers_action(s32 action)
{
    u32 *buffer;
    register u32 *image __asm__("$4");
    register u32 *palette __asm__("$3");
    register RrGarageTimBytes *tim __asm__("$5");
    u8 selected;
    volatile RrGarageSelectionPrefix *state;

    if (action < 0) {
        return;
    }
    if (action > 0) {
        switch (action) {
        case 1:
            garage_logo_state.selectorsB8[0] = 2;
            garage_logo_state.selectorsB8[3] = 2;
            garage_navigate_to_option(0);
            break;
        case 2:
            garage_logo_state.selectorsB8[0] = 1;
            garage_logo_state.selectorsB8[3] = 1;
            garage_navigate_to_option(0);
            break;
        case 4:
            garage_logo_state.selectorsB8[1] = 3;
            garage_logo_state.selectorsB8[2] = 3;
            garage_navigate_to_option(1);
            break;
        case 8:
            switch (garage_logo_state.optionIndices98.flat[0]) {
            case 1:
            case 2:
                garage_logo_state.optionIndices98.flat[3] = 0;
                /* Fall through: both paths select navigation row zero. */
            case 0:
            case 3:
                garage_logo_state.selectorsB8[1] = 0;
                break;
            case 4:
                garage_logo_state.selectorsB8[1] = 1;
                break;
            }
            garage_logo_state.selectorsB8[2] = 0;
            garage_navigate_to_option(1);
            break;
        }
    } else {
        if (garage_cursor_outside_button(15) == 0 &&
            (garage_logo_work.words[1] & 1)) {
            if (garage_logo_work.words[0] & 4) {
                garage_save_cursor_selection();
            } else {
                garage_logo_state.optionIndices98.flat[0] = 4;
                garage_logo_state.optionIndices98.flat[7] = 0;
                garage_logo_state.saved64[0] = 15;
                garage_logo_state.saved64[1] = 0;
                garage_logo_state.saved64[2] = 4;
                garage_logo_state.saved64[3] = 4;
                garage_logo_state.saved64[4] = 0;
            }
            garage_save_undo_copy();
            ((RrGarageButton *)garage_logo_buttons[15])->timer0E = 7;
            PlaySfx(1);

            /* Preserve the observed pointer access order after sound.
             * Register declarations retain allocation, emitting no instructions. */
            state = &garage_logo_state;
            selected = state->active21;
            buffer = state->buffers24[selected];
            tim = state->tim2C[selected];
            image = state->image40[selected];
            palette = state->paletteBackup50[selected];
            state->tim2C[selected] = (RrGarageTimBytes *)buffer;
            state->image40[selected] = state->image38[selected];
            buffer = state->palette48[selected];
            state->buffers24[selected] = (u32 *)tim;
            state->image38[selected] = image;
            state->palette48[selected] = palette;
            state->paletteBackup50[selected] = buffer;
        }
    }
}
