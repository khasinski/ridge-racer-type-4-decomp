#include "rr/course_choices.h"
extern RrCourseChoice4 versus_select_choice_tables[2][2][9];
extern s16 versus_select_choice_group[2],versus_select_choice_index[2];
void versus_select_apply_player_choice(void)
{
    ResetScreenCounters();
    LoadCursor = MenuArenas.first;
    UnpackChoiceGlyph((s8 *)&SelectionRows + SelectionPlayer * 4, versus_select_choice_tables[SelectionPlayer & 1][versus_select_choice_group[SelectionPlayer & 1]][versus_select_choice_index[SelectionPlayer & 1]].glyph);
    PlayerCourseChoices[SelectionPlayer].first = versus_select_choice_tables[SelectionPlayer & 1][versus_select_choice_group[SelectionPlayer & 1]][versus_select_choice_index[SelectionPlayer & 1]].first;
    PlayerCourseChoices[SelectionPlayer].second = versus_select_choice_tables[SelectionPlayer & 1][versus_select_choice_group[SelectionPlayer & 1]][versus_select_choice_index[SelectionPlayer & 1]].second;
    ImageRows[1] = 6;
}
