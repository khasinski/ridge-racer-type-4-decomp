#include "rr/overlay662_preview_drag.h"

void garage_func_801246E4(void) {
    u16 *grab;
    s32 offsetX;
    s32 thumb;
    s32 center;
    u16 nextX;
    u32 deltaY;

    if (garage_D_80127668.words[0] & 8) {
        if (garage_D_80127668.words[1] & 4) {
            garage_D_80127668.words[0] = (s32) (garage_D_80127668.words[0] & ~8);
            garage_func_8011E7C0(0xD, 0x3F, 0x126, 0x15A);
            return;
        }
        goto update_origin;
    }
    if (((u8) garage_D_801276A8.config1C >= 4U) && (garage_func_8011E6C8(0xED, 0x47, 0x40, 0x80) == 0) && (garage_D_80127668.words[1] & 1)) {
        garage_D_80127668.words[4] = 0x33;
        garage_D_80127668.words[0] = (s32) (garage_D_80127668.words[0] | 8);
        garage_func_8011E7C0(0xED, 0x47, 0x40, 0x80);
        thumb = 0x40 / (s32) ((u8) garage_D_801276A8.config1C >> 1);
        garage_D_80127294 = thumb;
        garage_D_80127298 = 0x40 - thumb;
        if (garage_func_8011E6C8((s16) (garage_D_801276A8.reset08[0] + 0xED), (s16) ((garage_D_801276A8.reset08[1] * 2) + 0x47), (s16) garage_D_80127294, thumb * 2) != 0) {
            grab=garage_D_80127290;
            center = (s32) garage_D_80127294 / 2;
            grab[1] = (u16) center;
            garage_D_80127290[0] = (u16) center;
        } else {
            garage_D_80127290[0] = (u16) ((garage_D_80127668.points.cursor14.x - 0xED) - garage_D_801276A8.reset08[0]);
            garage_D_80127290[1] = (u16) ((((s16) garage_D_80127668.points.cursor14.y / 2) - 0x23) - (u16) garage_D_801276A8.reset08[1]);
        }
update_origin:
        __asm__("" : : "m"(garage_D_801276A8),"m"(garage_D_80127668));
        offsetX=garage_D_80127290[0]+0xED;
        
        nextX = garage_D_80127668.points.cursor14.x - offsetX;
        garage_D_801276A8.reset08[0] = nextX;
        deltaY = (s16) garage_D_80127668.points.cursor14.y - 0x47;
        garage_D_801276A8.reset08[1] = (s16) (((s32) deltaY / 2) - garage_D_80127290[1]);
        if ((s16) nextX < 0) {
            garage_D_801276A8.reset08[0] = 0U;
        } else if (garage_D_80127298 < (s16) nextX) {
            garage_D_801276A8.reset08[0] = (u16) garage_D_80127298;
        }
        if (garage_D_801276A8.reset08[1] < 0) {
            garage_D_801276A8.reset08[1] = 0;
            return;
        }
        if (garage_D_80127298 < garage_D_801276A8.reset08[1]) {
            garage_D_801276A8.reset08[1] = (s16) (u16) garage_D_80127298;
        }
    }
}
