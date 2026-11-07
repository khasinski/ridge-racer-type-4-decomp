#include "rr/overlay662_scroll_drag.h"

void garage_func_80123CC8(void) {
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

    if (garage_D_80127668.words[0] & 8) {
        if (garage_D_80127668.words[1] & 4) {
            garage_D_80127668.words[0] = (s32) (garage_D_80127668.words[0] & ~8);
            garage_func_8011E7C0(0xD, 0x3F, 0x126, 0x15A);
            return;
        }
        goto update_position;
    }
    if (((u8) garage_D_801276A8.config1C >= 4U) && (garage_func_8011E6C8(0x5A, 0x151, 0x70, 8) == 0) && (garage_D_80127668.words[1] & 1)) {
        garage_D_80127668.words[4] = 0x2E;
        garage_D_80127668.words[0] = (s32) (garage_D_80127668.words[0] | 8);
        garage_func_8011E7C0(0x5A, 0x151, 0x70, 8);
        imageThumb = 0x40 / (s32) ((u8) garage_D_801276A8.config1C >> 1);
        __asm__("" : : : "memory");
        trackThumb = 0x70 / (s32) ((u8) garage_D_801276A8.config1C >> 1);
        imageTravel = 0x40 - imageThumb;
        trackTravel = 0x70 - trackThumb;
        projected = (s32) ((garage_D_80127668.points.cursor14.x - 0x5A) * imageTravel) / trackTravel;
        garage_D_8012726C = imageThumb;
        garage_D_80127270 = imageTravel;
        garage_D_80127274 = trackThumb;
        garage_D_80127278 = trackTravel;
        relative = projected - (s16) garage_D_801276A8.reset08[0];
        if ((relative < 0) || (relative >= imageThumb)) {
            garage_D_80127268 = (u16) ((s32) (((u32) trackThumb >> 1) * imageTravel) / trackTravel);
        } else {
            garage_D_80127268 = projected - garage_D_801276A8.reset08[0];
        }
update_position:
        coordinate=garage_D_8012767C-0x5A;
        limit = garage_D_80127270;
        selection=&garage_D_801276A8;
        narrowed = ((s32) (coordinate * limit) / (s32) garage_D_80127278) - garage_D_80127268;
        selection->reset08[0] = narrowed;
        if ((s16) narrowed < 0) {
            selection->reset08[0] = 0U;
            return;
        }
        if (limit < (s16) narrowed) {
            selection->reset08[0] = (u16) garage_D_80127270;
        }
    }
}
