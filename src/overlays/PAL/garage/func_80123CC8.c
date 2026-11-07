#include "rr/overlay662_scroll_drag.h"

void garage_drag_hscrollbar(void) {
    s32 imageTravel;
    s32 trackTravel;
    s32 limit;
    s32 imageThumb;
    s32 trackThumb;
    s32 projected;
    s32 relative;
    u16 narrowed;
    s32 coordinate;
    RrGarageSelectionPrefix *selection;

    if (garage_logo_work.words[0] & 8) {
        if (garage_logo_work.words[1] & 4) {
            garage_logo_work.words[0] = (s32) (garage_logo_work.words[0] & ~8);
            garage_set_cursor_bounds(0xD, 0x3F, 0x126, 0x15A);
            return;
        }
        goto update_position;
    }
    if (((u8) garage_logo_state.config1C >= 4U) && (garage_cursor_outside_rect(0x5A, 0x151, 0x70, 8) == 0) && (garage_logo_work.words[1] & 1)) {
        garage_logo_work.words[4] = 0x2E;
        garage_logo_work.words[0] = (s32) (garage_logo_work.words[0] | 8);
        garage_set_cursor_bounds(0x5A, 0x151, 0x70, 8);
        imageThumb = 0x40 / (s32) ((u8) garage_logo_state.config1C >> 1);
        __asm__("" : : : "memory");
        trackThumb = 0x70 / (s32) ((u8) garage_logo_state.config1C >> 1);
        imageTravel = 0x40 - imageThumb;
        trackTravel = 0x70 - trackThumb;
        projected = (s32) ((garage_logo_work.points.cursor14.x - 0x5A) * imageTravel) / trackTravel;
        garage_hscroll_image_thumb = imageThumb;
        garage_hscroll_image_travel = imageTravel;
        garage_hscroll_track_thumb = trackThumb;
        garage_hscroll_track_travel = trackTravel;
        relative = projected - (s16) garage_logo_state.reset08[0];
        if ((relative < 0) || (relative >= imageThumb)) {
            garage_hscroll_grab_offset = (u16) ((s32) (((u32) trackThumb >> 1) * imageTravel) / trackTravel);
        } else {
            garage_hscroll_grab_offset = projected - garage_logo_state.reset08[0];
        }
update_position:
        coordinate=garage_logo_cursor_x-0x5A;
        limit = garage_hscroll_image_travel;
        selection=&garage_logo_state;
        narrowed = ((s32) (coordinate * limit) / (s32) garage_hscroll_track_travel) - garage_hscroll_grab_offset;
        selection->reset08[0] = narrowed;
        if ((s16) narrowed < 0) {
            selection->reset08[0] = 0U;
            return;
        }
        if (limit < (s16) narrowed) {
            selection->reset08[0] = (u16) garage_hscroll_image_travel;
        }
    }
}
