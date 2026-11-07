/* CC1_FLAGS: -fno-schedule-insns */
#include "rr/overlay662_options.h"
#include "rr/overlay662_distance.h"

void garage_navigate_to_option(s32 mode) {
    RrGarageOptionRecord *options;
    s32 row;
    s32 col;
    s32 target;

    if (mode != 0) {
        s32 sel = garage_logo_state.selectorsB8[0];
        options = garage_logo_state.options78.groups.first[sel];
        col = garage_logo_state.selectorsB8[2];
        row = garage_logo_state.optionIndices98.groups.first[sel];
    } else {
        s32 sel = garage_logo_state.selectorsB8[1];
        options = garage_logo_state.options78.groups.second[sel];
        col = garage_logo_state.selectorsB8[3];
        row = garage_logo_state.optionIndices98.groups.second[sel];
    }
    target = *(col + options[row].destinations);
    garage_logo_work.points.second24 = garage_get_button_center(target);
    garage_logo_work.points.distance2C = garage_calc_glide_step(&garage_logo_work.points.first20, &garage_logo_work.points.second24);
    garage_logo_state.current60 = target;
}
