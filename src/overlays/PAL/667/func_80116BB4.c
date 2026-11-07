#include "rr/course_choices.h"
extern RrCourseChoice4 CourseMenuChoices[2][2][9];
extern s16 CourseMenuPage[2],CourseMenuCursor[2];
void ovl667_func_80116BB4(void) {
    ResetScreenCounters();
    LoadCursor = MenuArenas.first;
    UnpackChoiceGlyph(
        (s8 *)&SelectionRows + SelectionPlayer * 4,
        CourseMenuChoices[SelectionPlayer & 1][CourseMenuPage[SelectionPlayer & 1]][CourseMenuCursor[SelectionPlayer & 1]].glyph
    );
    PlayerCourseChoices[SelectionPlayer].first = CourseMenuChoices[SelectionPlayer & 1][CourseMenuPage[SelectionPlayer & 1]][CourseMenuCursor[SelectionPlayer & 1]].first;
    PlayerCourseChoices[SelectionPlayer].second = CourseMenuChoices[SelectionPlayer & 1][CourseMenuPage[SelectionPlayer & 1]][CourseMenuCursor[SelectionPlayer & 1]].second;
    ImageRows[1] = 6;
}
