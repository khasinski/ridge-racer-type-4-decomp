#include "rr/overlay665_visual.h"
void options_func_801169BC(s32 arg0)
{
    u16 pts[4];

    pts[0] = (rsin(arg0) / 128) + options_D_80119768.centerX38;
    pts[1] = (-rcos(arg0) / 64) + options_D_80119768.centerY3A;
    pts[2] = ((rsin(arg0) * 40) / 8192) + options_D_80119768.centerX38;
    pts[3] = ((-rcos(arg0) * 40) / 4096) + options_D_80119768.centerY3A;
    AddLineF2Prim((const s16 *)pts, options_D_8011973C, 0, 0);
}
