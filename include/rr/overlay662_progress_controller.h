#ifndef RR_OVERLAY662_PROGRESS_CONTROLLER_H
#define RR_OVERLAY662_PROGRESS_CONTROLLER_H
#include "rr/overlay662_setup.h"
#include "rr/menu_transition.h"
#include "rr/frame_environment.h"
#include "rr/tables.h"
#include "rr/overlay_menu.h"
extern RrMenuItemState5C PlayerControllers[2];
extern s16 MenuFadeTimer;
extern u16 garage_D_801272D8[4],garage_D_80127328[4][4];
s32 LoadGlyphRowsStep(void);
void garage_func_80116690(void);
void garage_func_80116808(void);
void garage_func_80116960(void);
void garage_func_8011512C(void);
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
void garage_func_801161FC(void);
#endif
