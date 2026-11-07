#include "rr/garage_rotate.h"
void garage_rotate_tool_action(s32 arg0) {
    RrGarageSelectionPrefix *selection;
    s32 mode;
    /* Native-width address cursor: the final unused increment may pass
     * the image end. Only valid in-buffer addresses become pointers. */
    unsigned long var_t5;
    s32 var_t2;
    s32 var_t4;
    s32 var_t6;
    u32 *var_t7;
    u32 *var_t8;
    s32 var_v0;
    s32 last;
    register s32 var_v1 __asm__("v1");
    u32 temp_a0;
    u32 temp_a1;
    u32 temp_a2;
    u32 temp_a3;
    u32 temp_t0;
    u32 temp_t1;
    u32 temp_v0;
    u32 temp_v1;
    u32 *temp_t3;

    var_v1 = arg0;
    if(var_v1 < 0) {
      garage_logo_state.optionIndices98.flat[0]=1;
      garage_logo_state.optionIndices98.flat[3]=0;
      goto done;
    }
    if(var_v1 <= 0) goto update;
    if(var_v1 == 2) goto key2;
    if(var_v1 < 3) {
        if(var_v1 == 1) goto key1;
        goto done;
    }
    if(var_v1 == 4) goto key4;
    if(var_v1 == 8) goto key8;
    goto done;
key1:
    garage_logo_state.optionIndices98.flat[0]=0;
    garage_logo_state.selectorsB8[3]=0;
    garage_navigate_to_option(0);
    goto done;
key2:
    garage_logo_state.optionIndices98.flat[0]=var_v1;
    garage_logo_state.selectorsB8[3]=var_v1;
    garage_navigate_to_option(0);
    goto done;
key4:
    mode=1;selection=&garage_logo_state;
    selection->selectorsB8[1]=var_v1;
    goto keyCommon;
key8:
    mode=1;selection=&garage_logo_state;
    var_v1=mode;
    selection->optionIndices98.flat[3]=var_v1;
keyCommon:
    selection->selectorsB8[2]=var_v1;
    garage_navigate_to_option(mode);
    goto done;
update:
        if ((garage_cursor_outside_button(1) == 0) && (garage_logo_work.words[1] & 1)) {
            if (garage_logo_work.words[0] & 4) {
                garage_save_cursor_selection();
            } else {
                garage_logo_state.optionIndices98.flat[0] = 1;
                garage_logo_state.optionIndices98.flat[3] = 0;
                garage_logo_state.saved64[0] = 1;
                garage_logo_state.saved64[1] = 0;
                garage_logo_state.saved64[2] = 0;
                garage_logo_state.saved64[3] = 0;
                garage_logo_state.saved64[4] = 1;
            }
            garage_save_undo_copy();
            ((RrGarageButton *)garage_logo_buttons[1])->timer0E = 7;
            PlaySfx(1);
            var_t6 = 0;
            if (garage_logo_state.layer23 == 0) {
                var_t8 = garage_logo_state.image38[garage_logo_state.active21 == 0];
                var_t7 = garage_logo_state.image38[garage_logo_state.active21];
            } else {
                var_t8 = garage_logo_state.image40[garage_logo_state.active21 == 0];
                var_t7 = garage_logo_state.image40[garage_logo_state.active21];
            }
            last=7;
            do {
                var_t4 = 0;
loop_24:
                var_v0 = last - var_t4;
                temp_t3 = var_t8 + ((var_v0 << 6) + var_t6);
                var_t5 = (unsigned long)(var_t7 + ((var_t6 << 6) + var_t4));
                var_t2 = 0;
loop_25:
                temp_v0 = (u32) temp_t3[0] >> var_t2;
                temp_v1 = (u32) temp_t3[8] >> var_t2;
                temp_a0 = (u32) temp_t3[16] >> var_t2;
                temp_a1 = (u32) temp_t3[24] >> var_t2;
                temp_a2 = (u32) temp_t3[32] >> var_t2;
                temp_a3 = (u32) temp_t3[40] >> var_t2;
                temp_t0 = (u32) temp_t3[48] >> var_t2;
                temp_t1 = (u32) temp_t3[56] >> var_t2;
                var_t2 += 4;
                *(u32 *)var_t5 = (temp_v0 << 0x1C) | ((temp_v1 & 0xF) << 0x18) | ((temp_a0 & 0xF) << 0x14) | ((temp_a1 & 0xF) << 0x10) | ((temp_a2 & 0xF) << 0xC) | ((temp_a3 & 0xF) << 8) | ((temp_t0 & 0xF) * 0x10) | (temp_t1 & 0xF);
                var_t5 += 32;
                if (var_t2 < 0x20) {
                    goto loop_25;
                }
                var_t4 += 1;
                if (var_t4 < 8) {
                    goto loop_24;
                }
                var_t6 += 1;
            } while (var_t6 < 8);
        }
        return;
done:;
}
