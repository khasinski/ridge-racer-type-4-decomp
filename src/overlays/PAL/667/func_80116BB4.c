#include "rr/course_choices.h"
extern RrCourseChoice4 D_8010BFD8[2][2][9];
extern s16 D_8010C0D8[2],CourseMenuCursor[2];
void ovl667_func_80116BB4(void) {
    ResetScreenCounters();
    LoadCursor = MenuArenas.first;
    UnpackChoiceGlyph(
        (s8 *)&SelectionRows + SelectionPlayer * 4,
        D_8010BFD8[SelectionPlayer & 1][D_8010C0D8[SelectionPlayer & 1]][CourseMenuCursor[SelectionPlayer & 1]].glyph
    );
    PlayerCourseChoices[SelectionPlayer].first = D_8010BFD8[SelectionPlayer & 1][D_8010C0D8[SelectionPlayer & 1]][CourseMenuCursor[SelectionPlayer & 1]].first;
    PlayerCourseChoices[SelectionPlayer].second = D_8010BFD8[SelectionPlayer & 1][D_8010C0D8[SelectionPlayer & 1]][CourseMenuCursor[SelectionPlayer & 1]].second;
    ImageRows[1] = 6;
}
