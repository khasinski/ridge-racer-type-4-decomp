#include "rr/overlay662_garage.h"

#define SEL garage_logo_state
#define CUR garage_logo_work.points.cursor14

/* Map the cursor inside the 128x256 image area (at 82,71) to image cells:
 * reset08[2..5] is the zoom window origin/extent centred on the cursor,
 * reset08[6..9] the single cell under the cursor. Both are clamped to the
 * 128/config1C grid. Outside the area everything is cleared. */
void garage_map_cursor_to_cell(void) {
    RrGaragePoint p;
    s32 size, max, zoom, x, y;

    if (garage_cursor_outside_rect(82, 71, 128, 256) != 0) {
        SEL.reset08[2] = 0;
        SEL.reset08[3] = 0;
        SEL.reset08[4] = 0;
        SEL.reset08[5] = 0;
        SEL.reset08[6] = 0;
        SEL.reset08[7] = 0;
        SEL.reset08[8] = 0;
        SEL.reset08[9] = 0;
        return;
    }
    size = SEL.config1C;
    zoom = SEL.config1D;
    SEL.reset08[4] = SEL.reset08[5] = zoom;
    p.x = CUR.x - 82;
    p.y = (CUR.y - 71) >> 1;
    x = (p.x - (size * SEL.config1D - size) / 2) / size;
    max = 128 / SEL.config1C - zoom;
    y = (p.y - (size * SEL.config1D - size) / 2) / size;
    SEL.reset08[2] = x;
    SEL.reset08[3] = y;
    if (SEL.reset08[2] < 0) {
        SEL.reset08[2] = 0;
    } else if (max < SEL.reset08[2]) {
        SEL.reset08[2] = max;
    }
    if (SEL.reset08[3] < 0) {
        SEL.reset08[3] = 0;
    } else if (max < SEL.reset08[3]) {
        SEL.reset08[3] = max;
    }

    size = SEL.config1C;
    p.x = CUR.x - 82;
    p.y = (CUR.y - 71) >> 1;
    SEL.reset08[6] = p.x / size;
    SEL.reset08[7] = p.y / size;
    SEL.reset08[8] = SEL.reset08[9] = 1;
    max = 128 / SEL.config1C - 1;
    if (SEL.reset08[6] < 0) {
        SEL.reset08[6] = 0;
    } else if (max < SEL.reset08[6]) {
        SEL.reset08[6] = max;
    }
    if (SEL.reset08[7] < 0) {
        SEL.reset08[7] = 0;
    } else if (max < SEL.reset08[7]) {
        SEL.reset08[7] = max;
    }
}
