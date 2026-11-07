#ifndef RR_OVERLAY_EXTRA_LABELS_H
#define RR_OVERLAY_EXTRA_LABELS_H
#include "rr/menu_label_render.h"
extern RrUiUvRect records_record_label;
extern s32 records_selected_record,records_course_unlock_levels[9];
#include "rr/overlay662_name_state.h"
extern s32 ScreenFrame;
s32 rcos(s32);
/* Row index names an accessible badge. The current unlock index is0..8 and
 * its count fits two digits. Eight label bytes are NUL or supported characters.
 * Helper return values are full native arena pointers. Globals remain live
 * across service calls; the row position and current label byte are captured. */
void *records_draw_record_labels(u32 *,void *,s32);
void garage_draw_name_entry_field(u32 *,void *,s32);
#endif
