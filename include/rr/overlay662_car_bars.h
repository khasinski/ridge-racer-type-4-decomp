#ifndef RR_OVERLAY662_CAR_BARS_H
#define RR_OVERLAY662_CAR_BARS_H
#include "rr/badge_rows.h"
#include "rr/sprite_emit.h"
#include "rr/menu_choice_types.h"
extern s16 garage_D_80127300[20];
extern RrUiUvRect garage_D_80126214;
extern RrMenuChoice4 SelectionRows;
/* Twenty availability bars; selected may lie outside the displayed range.
 * Services may replace the context and update subsequent status entries. */
void garage_func_80116C34(s32 selected);
#endif
