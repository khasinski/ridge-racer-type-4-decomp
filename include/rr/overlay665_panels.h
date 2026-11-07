#ifndef RR_OVERLAY665_PANELS_H
#define RR_OVERLAY665_PANELS_H
#include "rr/overlay665_menu.h"
#include "rr/menu_label_render.h"
#include "rr/projection_state.h"
extern RrUiUvRect options_D_80119028;
void options_func_80115C8C(void);
void options_func_80115CFC(void);
void DrawHudDigit(s32,s32,s32,s32);
void UpdateBadgeRowAnim(s32);
#ifndef RR_PROJECTION_SCRATCH
#define RR_PROJECTION_SCRATCH ((RrProjectionScratchPrefix *)0x1F800000U)
#endif
/* Writable aligned packet arenas and a full frame ordering context. Device
 * links serialize low24 address bits; software cursors keep native width. */
#endif
