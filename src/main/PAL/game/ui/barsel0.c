#include "types.h"
#include "m2c_macros.h"
#include "game/ui/barsel0.h"
#include "rr/badge_rows.h"
#include "game/ui/barsel0.h"

extern int SetMenuFrameOptions();
extern int DrawMenuStepArrows();
extern int UpdateDialogue();
extern int SkipDialoguePage();
extern int UpdateBadgeRowAnim();
extern int UpdateGlyphRow();
extern s16 CourseListIndex;
extern u8 CourseListSavedIndex;
extern void *Dialogue;

s32 CourseListMenuStep(void) {
    s32 var_s2;

    var_s2 = 0;
    if (M2C_FIELD(BadgeRows, s16 *, 0x10) == 3) {
        SkipDialoguePage(Dialogue);
        UpdateDialogue(M2C_FIELD(BadgeRows, s16 *, 8) + 0xA, M2C_FIELD(BadgeRows, s16 *, 0xA) + 0x28, Dialogue);
    }
    UpdateBadgeRowAnim(0);
    if (M2C_FIELD(Dialogue, s8 *, 4) >= 0x22) {
        if (M2C_FIELD(BadgeRows, s16 *, 0x10) == 0xB) {
            if (SelectionDialogueMode == 1) {
                M2C_FIELD(BadgeRows, s16 *, 0x10) = 0;
            } else {
                goto block_6;
            }
        } else {
block_6:
            if (M2C_FIELD(BadgeRows, s16 *, 0x10) < 4) {
                DrawMenuStepArrows(0xA0, 0xCC, 0x30, 0, (s32) CourseListIndex, 0, 4);
                var_s2 = UpdateGlyphRow(CourseListSavedIndex);
            }
        }
    }
    SetMenuFrameOptions(1, 1, 0);
    return var_s2;
}
