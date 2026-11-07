#include "rr/diagnostics.h"
#include "rr/course_choices.h"
#include "rr/save_progress.h"
#include "rr/frame_environment.h"
extern s16 versus_select_win_counts[2],versus_select_choice_index[2],versus_select_choice_group[2];
extern s16 SelectedCarIndex,NumCars,MenuCursor,MenuItemCount;
extern s32 NumLaps,versus_select_course_menu_limit;
extern RrCourseChoice4 versus_select_choice_tables[2][2][9];
extern char versus_select_player_names[24],versus_select_player2_name_str[];
extern const char versus_select_car_info_job_str[];
extern u16 ModeGroup;
extern s16 ModeId,ModeSubState;
void SetDispMask(s32 mode);
void ClearScreenOverlayState(void);
void CopyStringClearTail(char *destination,char *source);
void versus_select_init_selection_screen(s32 mode);
void SeqStartTrack(s32 mode);
void SetMenuFrameOptions(s32,s32,s32);
/* Both destination name slots contain a NUL within12 bytes; the saved name
 * is terminated within its first12 bytes. String-tail cleanup keeps that bound. */
void versus_select_init_versus_mode(void)
{
    s32 i;
    s32 j;

    SetDispMask(0);
    ClearScreenOverlayState();
    for (i = 0; i < 2; i++) {
        versus_select_choice_group[i] = 0;
        versus_select_choice_index[i] = 0;
        versus_select_win_counts[i] = 0;
    }
    SelectedCarIndex = 0;
    CourseMenuSelection = 0;
    CourseIndex = 0;
    NumLaps = 3;
    NumCars = 2;
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 9; j++) {
            versus_select_choice_tables[0][i][j] = CourseChoiceTable[i][j];
            versus_select_choice_tables[1][i][j] = CourseChoiceTable[0][j];
        }
    }
    CopyStringClearTail(versus_select_player_names, (char *)SaveProgress.prefix.progress.prefix00.bytes);
    CopyStringClearTail(versus_select_player_names + 12, versus_select_player2_name_str);
    SetupDrawDispEnvs(0, 0, 0, 2);
    versus_select_init_selection_screen(0);
    UnpackChoiceGlyph((s8 *)&SelectionRows, CourseChoiceTable[0][0].glyph);
    UnpackChoiceGlyph((s8 *)&SelectionRows + 4, CourseChoiceTable[0][0].glyph);
    SeqStartTrack(0);
    versus_select_course_menu_limit = 8;
    ModeGroup = 1;
    ModeId = 56;
    ModeSubState = 0;
    DebugPrintStub(versus_select_car_info_job_str);
    SetMenuFrameOptions(0, 1, 1);
}
