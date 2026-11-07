#include "rr/garage_input.h"

void garage_read_dpad_repeat(u32 *output)
{
    u32 result=0;
    if (PlayerControllers[0].type01 != 0x12) {
        if (PlayerControllers[0].buttons02 & 0x1000) {
            garage_dpad_up_repeat++;
            if ((s32)garage_dpad_up_repeat >= 47) garage_dpad_up_repeat=40;
            if (garage_dpad_up_repeat == 1 || garage_dpad_up_repeat == 40) result |= 1;
            *output=result;
        } else {
            garage_dpad_up_repeat=0;
        }
        if (PlayerControllers[0].buttons02 & 0x4000) {
            garage_dpad_down_repeat++;
            if ((s32)garage_dpad_down_repeat >= 47) garage_dpad_down_repeat=40;
            if (garage_dpad_down_repeat == 1 || garage_dpad_down_repeat == 40) result |= 2;
            *output=result;
        } else {
            garage_dpad_down_repeat=0;
        }
        if (PlayerControllers[0].buttons02 & 0x8000) {
            garage_dpad_left_repeat++;
            if ((s32)garage_dpad_left_repeat >= 47) garage_dpad_left_repeat=40;
            if (garage_dpad_left_repeat == 1 || garage_dpad_left_repeat == 40) result |= 4;
            *output=result;
        } else {
            garage_dpad_left_repeat=0;
        }
        if (PlayerControllers[0].buttons02 & 0x2000) {
            garage_dpad_right_repeat++;
            if ((s32)garage_dpad_right_repeat >= 47) garage_dpad_right_repeat=40;
            if (garage_dpad_right_repeat == 1 || garage_dpad_right_repeat == 40) result |= 8;
            *output=result;
        } else {
            garage_dpad_right_repeat=0;
        }
    }
}
