#include "rr/diagnostics.h"
#include "rr/overlay_menu_init.h"
#include "rr/menu_transition.h"
#include "rr/frame_environment.h"
#include "rr/save_progress.h"
#include "rr/course_choice_types.h"
#include "rr/string_tail_copy.h"
extern RrCourseChoice4 CourseChoiceTable[2][9];
void SeqStartTrack(s32 mode);
#include "rr/overlay662_progress_init.h"
extern const char garage_D_8011487C[];
void SetModelTexturePage(s32 mode);
void garage_func_80114B9C(void) {
    SetDispMask(0);
    ClearScreenOverlayState();
    garage_func_80114998();
    SetupDrawDispEnvs(0, 0, 0, 2);
    SetModelTexturePage(25);
    ModeGroup = 1;
    ModeId = 40;
    ModeSubState = 0;
    DebugPrintStub(garage_D_8011487C);
    if (SaveProgress.prefix.reserved50.words[0] == 1) {
        if (CourseChoiceTable[1][7].glyph >= 0) {
            CourseChoiceTable[1][8].glyph = 359;
            CourseChoiceTable[1][8].first = 1;
        }
    }
    SeqStartTrack(0);
}
