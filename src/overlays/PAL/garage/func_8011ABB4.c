#include "rr/garage_draw_area.h"
u8 *garage_add_draw_area(u32 *ordering, u32 *pkt, u32 x, u32 y, u16 w, u16 h) {
    RrGpuRectangleFields rect;
    s16 py;
    rect.x = garage_logo_state.halfC8 + x;
    py = garage_logo_state.halfCA + y;
    if (FrameEnvs[0].environment.display.y == FrameEnvs[1].environment.display.y) {
        rect.y = py;
    } else {
        rect.y = (u32)py + ((u32)*(volatile s32 *)&DrawBufIndex << 8);
    }
    rect.w = w;
    rect.h = h;
    SetDrawArea(pkt, &rect);
    *pkt = (*pkt & 0xFF000000) | (*ordering & 0xFFFFFF);
    *ordering = (*ordering & 0xFF000000) | ((unsigned long)pkt & 0xFFFFFF);
    return (u8 *)(pkt + 3);
}
