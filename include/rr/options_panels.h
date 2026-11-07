#ifndef RR_OPTIONS_PANELS_H
#define RR_OPTIONS_PANELS_H
#include "rr/options_menu.h"
#include "rr/menu_label_render.h"
#include "rr/projection_state.h"
extern RrUiUvRect options_brightness_swatch_uv;
void options_init_brightness_test(void);
void options_update_brightness_test(void);
void DrawHudDigit(s32,s32,s32,s32);
void UpdateBadgeRowAnim(s32);
#ifndef RR_PROJECTION_SCRATCH
#define RR_PROJECTION_SCRATCH ((RrProjectionScratchPrefix *)0x1F800000U)
#endif
/* Writable aligned packet arenas and a full frame ordering context. Device
 * links serialize low24 address bits; software cursors keep native width. */
#endif
