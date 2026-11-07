#include "rr/options_visual.h"
void options_draw_angle_marker(s32 arg0)
{
    u16 pts[4];

    pts[0] = (rsin(arg0) / 128) + options_menu_state.centerX38;
    pts[1] = (-rcos(arg0) / 64) + options_menu_state.centerY3A;
    pts[2] = ((rsin(arg0) * 40) / 8192) + options_menu_state.centerX38;
    pts[3] = ((-rcos(arg0) * 40) / 4096) + options_menu_state.centerY3A;
    AddLineF2Prim((const s16 *)pts, options_marker_line_color, 0, 0);
}
