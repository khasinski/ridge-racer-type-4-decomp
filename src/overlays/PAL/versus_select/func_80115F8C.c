#include "rr/course_choices.h"
extern RrCourseChoice4 versus_select_D_80118458[2][2][9];
extern s16 versus_select_D_80118450[2],versus_select_D_8011844C[2];
void versus_select_func_80115F8C(void)
{
    ResetScreenCounters();
    LoadCursor = MenuArenas.first;
    UnpackChoiceGlyph((s8 *)&SelectionRows + SelectionPlayer * 4, versus_select_D_80118458[SelectionPlayer & 1][versus_select_D_80118450[SelectionPlayer & 1]][versus_select_D_8011844C[SelectionPlayer & 1]].glyph);
    PlayerCourseChoices[SelectionPlayer].first = versus_select_D_80118458[SelectionPlayer & 1][versus_select_D_80118450[SelectionPlayer & 1]][versus_select_D_8011844C[SelectionPlayer & 1]].first;
    PlayerCourseChoices[SelectionPlayer].second = versus_select_D_80118458[SelectionPlayer & 1][versus_select_D_80118450[SelectionPlayer & 1]][versus_select_D_8011844C[SelectionPlayer & 1]].second;
    ImageRows[1] = 6;
}
