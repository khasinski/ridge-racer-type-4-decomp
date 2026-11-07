/* CC1_FLAGS: -fno-schedule-insns */
#include "rr/overlay662_options.h"
#include "rr/overlay662_distance.h"

void garage_func_8011E5C0(s32 mode) {
    RrGarageOptionRecord *options;
    s32 row;
    s32 col;
    s32 target;

    if (mode != 0) {
        s32 sel = garage_D_801276A8.selectorsB8[0];
        options = garage_D_801276A8.options78.groups.first[sel];
        col = garage_D_801276A8.selectorsB8[2];
        row = garage_D_801276A8.optionIndices98.groups.first[sel];
    } else {
        s32 sel = garage_D_801276A8.selectorsB8[1];
        options = garage_D_801276A8.options78.groups.second[sel];
        col = garage_D_801276A8.selectorsB8[3];
        row = garage_D_801276A8.optionIndices98.groups.second[sel];
    }
    target = *(col + options[row].destinations);
    garage_D_80127668.points.second24 = garage_func_8011DC74(target);
    garage_D_80127668.points.distance2C = garage_func_8011DBD8(&garage_D_80127668.points.first20, &garage_D_80127668.points.second24);
    garage_D_801276A8.current60 = target;
}
