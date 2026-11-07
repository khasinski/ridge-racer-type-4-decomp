#ifndef RR_OVERLAY_MENU_INIT_H
#define RR_OVERLAY_MENU_INIT_H
#include "rr/menu_arenas.h"
#include "rr/menu_setup.h"
#include "rr/menu_selection.h"
#include "rr/overlay_menu.h"
#include "rr/renderer_setup.h"
extern s32 records_extra_courses_enabled,records_record_kind,records_available_course_count;
extern s32 records_selected_record;
/* Requires two badge rows. The final flag belongs to row1, not to padding
 * beyond a single row; the badge helper may replace that array before return. */
void records_init_selection(s32 mode);
/* The loader may replace the second arena. Layout calls may replace the UI
 * root and local condition; both are read after all four rows are configured. */
void records_init_menu(void);
void options_func_80114FD8(void);
void extra_course_select_init_menu(void);
#endif
