#include "rr/garage_frame_step.h"

void garage_update_editor_frame(void) {
    ScreenFrame=(s32)((u32)ScreenFrame+1U);
    if (++PlayTimeFrames > 0x014996FFU) {
        PlayTimeFrames = 0x014996FFU;
    }
    garage_update_selection_pulse();
    garage_update_cursor();
    garage_update_actions(0);
    garage_draw_cursor_sprite();
    garage_draw_animated_panels();
    garage_add_editor_draw_env();
    garage_update_editor_overlays();
    garage_upload_editor_vram();
    GarageActiveBuffer = garage_logo_state.buffers24[garage_logo_state.active21];
    GarageLogoTimPtr = garage_logo_state.tim2C[garage_logo_state.active21];
}
