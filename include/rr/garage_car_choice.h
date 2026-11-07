#ifndef RR_GARAGE_CAR_CHOICE_H
#define RR_GARAGE_CAR_CHOICE_H
#include "rr/course_choice_types.h"
#include "rr/menu_choice.h"
#include "rr/menu_selection.h"
#include "rr/save_progress.h"
#include "rr/menu_transition.h"
#include "rr/string_tail_copy.h"
#include "rr/menu_glyph.h"
#include "rr/diagnostics.h"
extern RrCourseChoice4 CourseChoiceTable[2][9];
extern s16 garage_car_index,garage_logo_number,garage_clut_choice;
extern char garage_player_name_copy[12];
extern const char garage_car_info_scene_name[];
void garage_init_car_info(void);
void garage_init_car_select(void);
/* Choice index0..8, signed byte coordinates, full nine-entry group. Saved and
 * destination names have accessible NUL terminators within12 bytes. */
void garage_enter_car_info(void);
void garage_enter_car_select(void);
#endif
