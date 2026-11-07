#ifndef RR_GARAGE_PROGRESS_CONTROLLER_H
#define RR_GARAGE_PROGRESS_CONTROLLER_H
#include "rr/garage_setup.h"
#include "rr/menu_transition.h"
#include "rr/frame_environment.h"
#include "rr/tables.h"
#include "rr/overlay_menu.h"
extern RrMenuItemState5C PlayerControllers[2];
extern s16 MenuFadeTimer;
extern u16 garage_team_car_counts[4],garage_team_maker_car_counts[4][4];
s32 LoadGlyphRowsStep(void);
void garage_step_team_cursor(void);
void garage_step_maker_cursor(void);
void garage_init_car_list(void);
void garage_init_car_info(void);
void AddScreenFadeTiles(u32 *,s32,u32);
void DrawUiRow(s32);
void UpdateBadgeRowAnim(s32);
void UpdateScreenFade(s32);
/* Entry phase must be0,1,2,3,4 or8. PAL table slots5..7 and its default
 * consume an unspecified saved register and are not controller states.
 * Choice bytes0/1 remain0..3 across callbacks. Published roots cover a complete
 * UI row, badge, four glyph rows and an enclosing frame ordering owner.
 * All services may change globals; the badge state is captured before input.
 * Nonzero glyph-step completion triggers navigation. Countdown wraps signed16. */
void garage_update_collection_select(void);
#endif
