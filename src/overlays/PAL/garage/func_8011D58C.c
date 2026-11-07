#include "rr/garage_garage.h"
#include "rr/tables.h"
extern RrMenuItemState5C PlayerControllers[];
extern void *PsyqMemset(void *, s32, u32);
void garage_read_mouse_motion(RrGaragePoint *output)
{
    RrGaragePoint point;
    s32 value;
    s32 y;
    PsyqMemset(&point,0,4);
    if (PlayerControllers[1].type01 == 0x12) {
        value = (s32)((u32)PlayerControllers[1].axis14 << 24) >> 25;
        if (value < -3 || value > 3) value += value / 2;
        if (value) point.x=value;
        y = PlayerControllers[1].axis15;
        if (y < -7 || y > 7) y += (y + (s32)((u32)y >> 31)) >> 1;
        if (y) point.y=y;
    }
    if ((u16)point.x | (u16)point.y) {
        output->x=point.x;
        output->y=point.y;
    }
}
