#include "rr/overlay662_input.h"
void garage_read_analog_motion(RrGaragePoint *output)
{
    RrGaragePoint point;
    s32 step,delta;
    u32 axis;
    PsyqMemset(&point,0,4);
    if (PlayerControllers[0].type01 == 0x73 && PlayerControllers[0].kind1C != 0 && PlayerControllers[0].flag40 == 0) {
        step=1;
        if (PlayerControllers[0].buttons02 & 8) step=4;
        axis=PlayerControllers[0].axis16;
        if (axis <112) {
            delta=((s32)axis-112)*step;
            if (garage_analog_cursor_remainder[0] <0) garage_analog_cursor_remainder[0]+=delta;
            else garage_analog_cursor_remainder[0]=delta;
            point.x=garage_analog_cursor_remainder[0]/112;
            garage_analog_cursor_remainder[0]-=point.x*112;
        } else if (axis>=145) {
            delta=((s32)axis-143)*step;
            if (garage_analog_cursor_remainder[0] >0) garage_analog_cursor_remainder[0]+=delta;
            else garage_analog_cursor_remainder[0]=delta;
            point.x=garage_analog_cursor_remainder[0]/112;
            garage_analog_cursor_remainder[0]-=point.x*112;
        } else garage_analog_cursor_remainder[0]=0;
        axis=PlayerControllers[0].axis17;
        if (axis <112) {
            delta=((s32)axis-112)*step;
            if (garage_analog_cursor_remainder[1] <0) garage_analog_cursor_remainder[1]+=delta;
            else garage_analog_cursor_remainder[1]=delta;
            point.y=(s16)(garage_analog_cursor_remainder[1]/112)*2;
            garage_analog_cursor_remainder[1]-=point.y*112;
        } else if (axis>=145) {
            delta=((s32)axis-143)*step;
            if (garage_analog_cursor_remainder[1] >0) garage_analog_cursor_remainder[1]+=delta;
            else garage_analog_cursor_remainder[1]=delta;
            point.y=(s16)(garage_analog_cursor_remainder[1]/112)*2;
            garage_analog_cursor_remainder[1]-=point.y*112;
        } else garage_analog_cursor_remainder[1]=0;
    }
    if ((u16)point.x | (u16)point.y) {output->x=point.x;output->y=point.y;}
}
