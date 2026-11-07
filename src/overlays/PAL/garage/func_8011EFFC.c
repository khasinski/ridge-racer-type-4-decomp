#include "rr/garage_mirror.h"
void garage_mirror_tool_action(s32 arg0) {
    s32 last;
    s32 temp_a1;
    s32 var_a2;
    u32 *var_t0;
    s32 var_t2;
    u32 *var_a3;
    u32 *var_t1;
    u32 temp_a0;

    if(arg0<0) {
        garage_logo_state.optionIndices98.flat[0] = 1;
        garage_logo_state.optionIndices98.flat[3] = 1;
        return;
    }
    if(arg0<=0) goto update;
    if(arg0==2) goto key2;
    if(arg0<3) {if(arg0==1) goto key1;goto done;}
    if(arg0==4) goto key4;
    if(arg0==8) goto key8;
    goto done;
key1:
        garage_logo_state.optionIndices98.flat[0] = 0;
        garage_logo_state.selectorsB8[3] = 0;
        garage_navigate_to_option(0);
        return;
key2:
        garage_logo_state.optionIndices98.flat[0] = 2;
        garage_logo_state.selectorsB8[3] = 2;
        garage_navigate_to_option(0);
        return;
key4:
        garage_logo_state.optionIndices98.flat[3] = 0;
        garage_logo_state.selectorsB8[2] = 0;
        garage_navigate_to_option(1);
        return;
key8:
        garage_logo_state.selectorsB8[1] = 2;
        garage_logo_state.selectorsB8[2] = 2;
        garage_navigate_to_option(1);
        return;
update:
        if ((garage_cursor_outside_button(2) == 0) && (garage_logo_work.words[1] & 1)) {
            if (garage_logo_work.words[0] & 4) {
                garage_save_cursor_selection();
            } else {
                garage_logo_state.optionIndices98.flat[0] = 0;
                garage_logo_state.optionIndices98.flat[3] = 1;
                garage_logo_state.saved64[0] = 2;
                garage_logo_state.saved64[1] = 0;
                garage_logo_state.saved64[2] = 0;
                garage_logo_state.saved64[3] = 1;
                garage_logo_state.saved64[4] = 1;
            }
            garage_save_undo_copy();
            ((RrGarageButton *)garage_logo_buttons[2])->timer0E = 7;
            PlaySfx(1);
            var_t2 = 0;
            if (garage_logo_state.layer23 == 0) {
                var_t1 = garage_logo_state.image38[garage_logo_state.active21 == 0];
                var_t0 = garage_logo_state.image38[garage_logo_state.active21];
            } else {
                var_t1 = garage_logo_state.image40[garage_logo_state.active21 == 0];
                var_t0 = garage_logo_state.image40[garage_logo_state.active21];
            }
            last=7;
            do {
                var_a2 = 0;
                var_a3 = var_t1;
loop_23:
                temp_a0 = *var_a3;
                var_a3 += 1;
                temp_a1 = last - var_a2;
                var_a2 += 1;
                var_t0[temp_a1] = (temp_a0 << 0x1C) | (((temp_a0 >> 4) & 0xF) << 0x18) | (((temp_a0 >> 8) & 0xF) << 0x14) | (((temp_a0 >> 0xC) & 0xF) << 0x10) | (((temp_a0 << 12) >> 16) & 0xF000) | (((temp_a0 << 8) >> 20) & 0xF00) | ((temp_a0 >> 0x14) & 0xF0) | (temp_a0 >> 0x1C);
                if (var_a2 < 8) {
                    goto loop_23;
                }
                var_t2 += 1;
                var_t1 += 8;
                var_t0 += 8;
            } while (var_t2 < 0x40);
        }
        return;
done:;
}
