#ifndef RR_OVERLAY662_CAR_BARS_H
#define RR_OVERLAY662_CAR_BARS_H
#include "rr/badge_rows.h"
#include "rr/sprite_emit.h"
#include "rr/menu_choice_types.h"
extern s16 garage_car_status[20];
extern RrUiUvRect garage_owned_car_marker_sprite;
extern RrMenuChoice4 SelectionRows;
/* Twenty availability bars; selected may lie outside the displayed range.
 * Services may replace the context and update subsequent status entries. */
void garage_draw_car_status_bars(s32 selected);
#endif
