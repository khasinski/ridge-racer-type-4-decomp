#ifndef RR_GARAGE_UPDATE_H
#define RR_GARAGE_UPDATE_H
#include "rr/garage_render_step.h"
#include "rr/menu_background.h"
#include "rr/garage_garage.h"
#include "rr/frame_environment.h"
#include "rr/screen_tiles.h"
#include "rr/menu_text_state.h"
void UpdateBadgeRowAnim(s32 index);
void DrawUiRow(s32 index);
void SetMenuFrameOptions(s32 first,s32 second,s32 third);
/* D_ACC14 points at the first rectangle of a containing frame owner.
 * Callback writes are visible to every later flag, fade and text-root read. */
#endif
