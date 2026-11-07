#ifndef RR_OVERLAY_MENU_H
#define RR_OVERLAY_MENU_H
#include "rr/menu_choice.h"

extern s16 MenuCursor,MenuItemCount;
void SetMenuTextRowCursor(s32 row,s32 first,s32 second);
void StepMenuValue(s32 mode,s32 width,char *selection,s32 limit,s32 flags);
void DrawMenuStepArrows(s32 x,s32 y,s32 width,s32 mode,s32 selected,s32 first,s32 second);
s32 UpdateGlyphRow(s32 selected);

s32 time_attack_course_select_update_results_choice(s32 reserved);
s32 versus_select_update_results_choice(void);
s32 extra_course_select_update_retry_choice(s32 mode);
/* Four-position selection, backed by at least three UI rows and four glyph
 * rows. A sound callback may replace the roots; its signed selection is clamped. */
void extra_course_select_update_course_selection(void);
s32 serial_link_update_retry_choice(void);
/* The first machine-word argument is passed by callers but unused.
 * Return the final glyph-step completion value without truncation. */
s32 garage_draw_select_arrows(u32 unused,s32 mode);
#endif
