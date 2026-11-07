#include "rr/garage_preview_drag.h"

void garage_drag_preview_viewport(void) {
    u16 *grab;
    s32 offsetX;
    s32 thumb;
    s32 center;
    u16 nextX;
    u32 deltaY;

    if (garage_logo_work.words[0] & 8) {
        if (garage_logo_work.words[1] & 4) {
            garage_logo_work.words[0] = (s32) (garage_logo_work.words[0] & ~8);
            garage_set_cursor_bounds(0xD, 0x3F, 0x126, 0x15A);
            return;
        }
        goto update_origin;
    }
    if (((u8) garage_logo_state.config1C >= 4U) && (garage_cursor_outside_rect(0xED, 0x47, 0x40, 0x80) == 0) && (garage_logo_work.words[1] & 1)) {
        garage_logo_work.words[4] = 0x33;
        garage_logo_work.words[0] = (s32) (garage_logo_work.words[0] | 8);
        garage_set_cursor_bounds(0xED, 0x47, 0x40, 0x80);
        thumb = 0x40 / (s32) ((u8) garage_logo_state.config1C >> 1);
        garage_preview_drag_thumb = thumb;
        garage_preview_drag_travel = 0x40 - thumb;
        if (garage_cursor_outside_rect((s16) (garage_logo_state.reset08[0] + 0xED), (s16) ((garage_logo_state.reset08[1] * 2) + 0x47), (s16) garage_preview_drag_thumb, thumb * 2) != 0) {
            grab=garage_preview_drag_grab;
            center = (s32) garage_preview_drag_thumb / 2;
            grab[1] = (u16) center;
            garage_preview_drag_grab[0] = (u16) center;
        } else {
            garage_preview_drag_grab[0] = (u16) ((garage_logo_work.points.cursor14.x - 0xED) - garage_logo_state.reset08[0]);
            garage_preview_drag_grab[1] = (u16) ((((s16) garage_logo_work.points.cursor14.y / 2) - 0x23) - (u16) garage_logo_state.reset08[1]);
        }
update_origin:
        __asm__("" : : "m"(garage_logo_state),"m"(garage_logo_work));
        offsetX=garage_preview_drag_grab[0]+0xED;
        
        nextX = garage_logo_work.points.cursor14.x - offsetX;
        garage_logo_state.reset08[0] = nextX;
        deltaY = (s16) garage_logo_work.points.cursor14.y - 0x47;
        garage_logo_state.reset08[1] = (s16) (((s32) deltaY / 2) - garage_preview_drag_grab[1]);
        if ((s16) nextX < 0) {
            garage_logo_state.reset08[0] = 0U;
        } else if (garage_preview_drag_travel < (s16) nextX) {
            garage_logo_state.reset08[0] = (u16) garage_preview_drag_travel;
        }
        if (garage_logo_state.reset08[1] < 0) {
            garage_logo_state.reset08[1] = 0;
            return;
        }
        if (garage_preview_drag_travel < garage_logo_state.reset08[1]) {
            garage_logo_state.reset08[1] = (s16) (u16) garage_preview_drag_travel;
        }
    }
}
