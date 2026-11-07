#ifndef RR_OVERLAY662_CAR_CHOICE_H
#define RR_OVERLAY662_CAR_CHOICE_H
#include "rr/course_choice_types.h"
#include "rr/menu_choice.h"
#include "rr/menu_selection.h"
#include "rr/save_progress.h"
#include "rr/menu_transition.h"
#include "rr/string_tail_copy.h"
#include "rr/menu_glyph.h"
#include "rr/diagnostics.h"
extern RrCourseChoice4 CourseChoiceTable[2][9];
extern s16 garage_D_801272E4,garage_D_801272BC,garage_D_801272E2;
extern char garage_D_801272C8[12];
extern const char garage_D_80114888[];
void garage_func_8011512C(void);
void garage_func_80118ED8(void);
/* Choice index0..8, signed byte coordinates, full nine-entry group. Saved and
 * destination names have accessible NUL terminators within12 bytes. */
void garage_func_801153B0(void);
void garage_func_80118E6C(void);
#endif
