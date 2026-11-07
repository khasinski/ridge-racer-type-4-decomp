#ifndef RR_COURSE_CHOICES_H
#define RR_COURSE_CHOICES_H
#include "rr/layout.h"
#include "rr/menu_glyph.h"
#include "rr/menu_selection.h"
#include "rr/menu_arena_types.h"
#include "rr/selection.h"
#include "rr/course_choice_types.h"
extern RrCourseChoice4 CourseChoiceTable[2][9];
extern RrMenuArenas MenuArenas;
extern void *LoadCursor;
extern u8 *ImageRows;
void ResetScreenCounters(void);
/* Nine readable entries; glyph-1 terminates. The final glyph0x167 counts
 * as the ninth valid entry. */
s32 CountChoiceGlyphs(const RrCourseChoice4 *choices);
/* Fresh player0..3, two-way group selector and course0..8 are required.
 * Scene setup may replace roots and selections before the copies are made.
 * The published UI pointer supplies at least two writable bytes. */
void time_attack_course_select_func_801163C4(void);
void versus_select_func_80115F8C(void);
void serial_link_func_80116BB4(void);
RR_ASSERT_SIZE(RrCourseChoice4,4);
RR_ASSERT_OFFSET(RrCourseChoice4,second,2);
RR_ASSERT_OFFSET(RrCourseChoice4,first,3);
#endif
