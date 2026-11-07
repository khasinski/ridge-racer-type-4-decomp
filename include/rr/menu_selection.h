#ifndef RR_MENU_SELECTION_H
#define RR_MENU_SELECTION_H
#include "rr/layout.h"
extern u8 D_8009F64C[8];
extern s16 ReverseCourse,CourseMenuSelection,CourseIndex;
extern s32 SelectionPlayer;
/* Word result0..255: the original loads the byte with zero extension. */
s32 DecodeCourseIndex(s32 selection);
typedef struct {s16 first,second;} RrMenuGridPoint;
extern RrMenuGridPoint PlayerCourseChoices[4];
RR_ASSERT_SIZE(RrMenuGridPoint,4);
RR_ASSERT_OFFSET(RrMenuGridPoint,second,2);
#endif
