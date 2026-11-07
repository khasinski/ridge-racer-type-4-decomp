#include "types.h"
#include "m2c_macros.h"
#include "game/ui/barsel1.h"
#include "rr/badge_rows.h"
#include "game/ui/barsel0.h"
#include "game/data/selection.h"

extern int SetMenuFrameOptions();
extern int DrawMenuStepArrows();
extern int UpdateDialogue();
extern int SkipDialoguePage();
extern int UpdateBadgeRowAnim();
extern int UpdateGlyphRow();
extern s16 SelectedTeamIndex;
extern void *Dialogue;

s32 TeamSelectMenuStep(void) {
    s32 var_s2;

    var_s2 = 0;
    if (M2C_FIELD(BadgeRows, s16 *, 0x10) == 3) {
        SkipDialoguePage(Dialogue);
        UpdateDialogue(M2C_FIELD(BadgeRows, s16 *, 8) + 0xA, M2C_FIELD(BadgeRows, s16 *, 0xA) + 0x28, Dialogue);
    }
    UpdateBadgeRowAnim(0);
    if (M2C_FIELD(Dialogue, s8 *, 4) >= 0x22) {
        if (M2C_FIELD(BadgeRows, s16 *, 0x10) == 0xB) {
            if (SelectionDialogueMode == 0) {
                M2C_FIELD(BadgeRows, s16 *, 0x10) = 0;
            } else {
                goto block_6;
            }
        } else {
block_6:
            if (M2C_FIELD(BadgeRows, s16 *, 0x10) < 4) {
                DrawMenuStepArrows(0xA0, 0xCC, 0x30, 0, (s32) SelectedTeamIndex, 0, 4);
                var_s2 = UpdateGlyphRow(SelectionRows[0]);
            }
        }
    }
    SetMenuFrameOptions(1, 1, 0);
    return var_s2;
}
