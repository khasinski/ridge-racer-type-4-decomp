#include "rr/garage_scroll_drag.h"

void garage_drag_vscrollbar(void) {
    s32 imageTravel;
    s32 trackTravel;
    s32 limit;
    s32 imageThumb;
    s32 trackThumb;
    s32 projected;
    s32 relative;
    u16 narrowed;
    RrGarageSelectionPrefix *selection;
    s32 coordinate;

    if (garage_logo_work.words[0] & 8) {
        if (garage_logo_work.words[1] & 4) {
            garage_logo_work.words[0] = (s32) (garage_logo_work.words[0] & ~8);
            garage_set_cursor_bounds(0xD, 0x3F, 0x126, 0x15A);
            return;
        }
        goto update_position;
    }
    if (((u8) garage_logo_state.config1C >= 4U) && (garage_cursor_outside_rect(0xD8, 0x57, 4, 0xE0) == 0) && (garage_logo_work.words[1] & 1)) {
        garage_logo_work.words[4] = 0x2F;
        garage_logo_work.words[0] = (s32) (garage_logo_work.words[0] | 8);
        garage_set_cursor_bounds(0xD8, 0x57, 4, 0xE0);
        imageThumb = 0x40 / (s32) ((u8) garage_logo_state.config1C >> 1);
        __asm__("" : : : "memory");
        trackThumb = 0xE0 / (s32) ((u8) garage_logo_state.config1C >> 1);
        imageTravel = 0x40 - imageThumb;
        trackTravel = 0xE0 - trackThumb;
        projected = (s32) ((garage_logo_work.points.cursor14.y - 0x57) * imageTravel) / trackTravel;
        garage_vscroll_image_thumb = imageThumb;
        garage_vscroll_image_travel = imageTravel;
        garage_vscroll_track_thumb = trackThumb;
        garage_vscroll_track_travel = trackTravel;
        relative = projected - (s16) garage_logo_state.reset08[1];
        if ((relative < 0) || (relative >= imageThumb)) {
            garage_vscroll_grab_offset = (u16) ((s32) (((u32) trackThumb >> 1) * imageTravel) / trackTravel);
        } else {
            garage_vscroll_grab_offset = projected - garage_logo_state.reset08[1];
        }
update_position:
        coordinate=garage_logo_cursor_y-0x57;
        limit = garage_vscroll_image_travel;
        selection=&garage_logo_state;
        narrowed = ((s32) (coordinate * limit) / (s32) garage_vscroll_track_travel) - garage_vscroll_grab_offset;
        selection->reset08[1] = narrowed;
        if ((s16) narrowed < 0) {
            selection->reset08[1] = 0U;
            return;
        }
        if (limit < (s16) narrowed) {
            selection->reset08[1] = (u16) garage_vscroll_image_travel;
        }
    }
}
