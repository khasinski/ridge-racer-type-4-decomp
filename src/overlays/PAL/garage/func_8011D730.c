#include "rr/garage_input.h"

void garage_read_analog_repeat(u32 *output)
{
    u32 result=0;
    s32 axis;
    if (PlayerControllers[0].type01 == 0x73 && PlayerControllers[0].kind1C != 0 && PlayerControllers[0].flag40 == 0) {
        axis=PlayerControllers[0].axis17;
        if (axis < 32) {
            garage_analog_up_repeat++;
            if ((s32)garage_analog_up_repeat >= 47) garage_analog_up_repeat=40;
            if (garage_analog_up_repeat == 1 || garage_analog_up_repeat == 40) result |= 1;
            *output=result;
        } else {
            garage_analog_up_repeat=0;
        }
        if (axis >= 225) {
            garage_analog_down_repeat++;
            if ((s32)garage_analog_down_repeat >= 47) garage_analog_down_repeat=40;
            if (garage_analog_down_repeat == 1 || garage_analog_down_repeat == 40) result |= 2;
            *output=result;
        } else {
            garage_analog_down_repeat=0;
        }
        axis=PlayerControllers[0].axis16;
        if (axis < 32) {
            garage_analog_left_repeat++;
            if ((s32)garage_analog_left_repeat >= 47) garage_analog_left_repeat=40;
            if (garage_analog_left_repeat == 1 || garage_analog_left_repeat == 40) result |= 4;
            *output=result;
        } else {
            garage_analog_left_repeat=0;
        }
        if (axis >= 225) {
            garage_analog_right_repeat++;
            if ((s32)garage_analog_right_repeat >= 47) garage_analog_right_repeat=40;
            if (garage_analog_right_repeat == 1 || garage_analog_right_repeat == 40) result |= 8;
            *output=result;
        } else {
            garage_analog_right_repeat=0;
        }
    }
}
