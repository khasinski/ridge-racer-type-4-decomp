#include "rr/garage_cursor_control.h"

void garage_update_cursor(void) {
    RrGaragePoint movement;
    RrGaragePoint destination;
    u32 events;
    s32 flags;
    u16 ySum;
    u16 xSum;

    PsyqMemset(&movement, 0, 4);
    events = 0;
    if (!(garage_logo_work.words[0] & 1)) {
        if (garage_logo_work.words[0] & 2) {
            garage_update_cursor_glide();
            if ((s32)garage_logo_work.points.word28 >= 0x1000) {
                garage_logo_work.points.cursor18 = garage_logo_work.points.cursor14;
                garage_logo_work.words[0] &= ~2U;
                garage_logo_work.words[0] &= ~4U;
                goto processInput;
            }
        } else {
processInput:
            if (garage_logo_work.words[0] & 4) {
                if (!(garage_logo_work.words[0] & 8)) {
                    garage_update_cursor_glide();
                    if ((s32)garage_logo_work.points.word28 >= 0x1000) {
                        garage_read_analog_repeat(&events);
                        garage_read_dpad_repeat(&events);
                        garage_dispatch_direction_input(events);
                        goto finishInput;
                    }
                } else {
                    goto finishInput;
                }
            } else {
                garage_read_analog_motion(&movement);
                garage_read_mouse_motion(&movement);
                garage_read_dpad_motion(&movement);
                xSum = garage_logo_work.points.cursor18.x + movement.x;
                ySum = garage_logo_work.points.cursor18.y + movement.y;
                garage_logo_work.points.cursor18.x = xSum;
                garage_logo_work.points.cursor18.y = ySum;
                if ((s16) xSum < garage_logo_work.halves[24]) {
                    garage_logo_work.points.cursor18.x = (u16) garage_logo_work.halves[24];
                }
                if (((garage_logo_work.halves[24] + garage_logo_work.halves[26]) - 1) < (s16) garage_logo_work.points.cursor18.x) {
                    garage_logo_work.points.cursor18.x = (u16) (((u16) garage_logo_work.halves[24] + (u16) garage_logo_work.halves[26]) - 1);
                }
                if ((s16) ySum < garage_logo_work.halves[25]) {
                    garage_logo_work.points.cursor18.y = (u16) garage_logo_work.halves[25];
                }
                if (((garage_logo_work.halves[25] + garage_logo_work.halves[27]) - 1) < (s16) garage_logo_work.points.cursor18.y) {
                    garage_logo_work.points.cursor18.y = (u16) (((u16) garage_logo_work.halves[25] + (u16) garage_logo_work.halves[27]) - 1);
                }
                garage_logo_work.points.cursor14 = garage_logo_work.points.cursor18;
                flags = garage_logo_work.words[0];
                if (!(flags & 8) && (PlayerControllers[0].type01 != 0x12) && (PlayerControllers[0].buttons08 & 4)) {
                    garage_logo_work.words[0] = (s32) (flags | 4);
                    garage_logo_work.words[1] = 0x30;
                    garage_logo_work.points.first20 = garage_logo_work.points.cursor18;
                    destination=garage_get_button_center(garage_logo_state.saved64[0]);
                    garage_logo_work.points.second24 = destination;
                    garage_logo_work.points.word28 = 0;
                    garage_logo_work.points.distance2C = garage_calc_glide_step(&garage_logo_work.points.first20,&garage_logo_work.points.second24);
                    garage_logo_state.current60 = (s32) garage_logo_state.saved64[0];
                    garage_logo_state.selectorsB8[0] = (s32) garage_logo_state.saved64[1];
                    garage_logo_state.selectorsB8[1] = (s32) garage_logo_state.saved64[2];
                    garage_logo_state.selectorsB8[2] = (s32) garage_logo_state.saved64[3];
                    garage_logo_state.selectorsB8[3] = (s32) garage_logo_state.saved64[4];
                    garage_logo_actions[((RrGarageButton *)garage_logo_buttons[garage_logo_state.saved64[0]])->action0A]->callback(-1);
                    PlaySfx(3);
                    return;
                }
finishInput:
                garage_update_button_state();
            }
        }
    }
}
