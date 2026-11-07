#include "rr/garage_input.h"

void garage_read_dpad_motion(RrGaragePoint *output)
{
    RrGaragePoint point;
    u32 buttons;
    s32 step;
    PsyqMemset(&point,0,4);
    if (PlayerControllers[0].type01 != 0x12) {
        buttons = PlayerControllers[0].buttons02;
        step=1;
        if (buttons & 8) step=4;
        if (buttons & 0x1000) point.y=-(step*2);
        if (buttons & 0x4000) point.y=step*2;
        if (buttons & 0x8000) point.x=-step;
        if (buttons & 0x2000) point.x=step;
    }
    if ((u16)point.x | (u16)point.y) {
        output->x=point.x;
        output->y=point.y;
    }
}
