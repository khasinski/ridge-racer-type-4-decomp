#include "rr/menu_text_rows.h"
#include "rr/overlay_menu.h"

extern s16 MenuCursor;
extern s16 MenuItemCount;
extern s32 versus_select_results_text_ids[22];
extern s32 TotalRaceTime;
extern s16 versus_select_finish_ranks[2];
extern s16 versus_select_result_order[2];
extern s16 versus_select_win_counts[2];
extern s32 ModeFrame;

extern void SetMenuFrameOptions(s32, s32, s32);

void versus_select_init_results(void) {
    s32 i;

    InitMenuTextRows(SharedWorkBuffer);
    MenuCursor = 0;
    MenuItemCount = 3;
    for (i = 0; i < 11; i++) {
        SetMenuTextRow(i, i, 0, 0, versus_select_results_text_ids[i]);
    }
    for (i = 11; i < 22; i++) {
        SetMenuTextRow(i, i - 11, 0, 0, -1);
    }
    for (i = 33; i < 44; i++) {
        SetMenuTextRow(i, i - 33, 2, 0, versus_select_results_text_ids[i - 22]);
    }
    if (TotalRaceTime >= 0) {
        if (versus_select_finish_ranks[0] > versus_select_finish_ranks[1]) {
            versus_select_result_order[0] = 1;
            versus_select_result_order[1] = 0;
        } else {
            versus_select_result_order[0] = 0;
            versus_select_result_order[1] = 1;
        }
        if (versus_select_win_counts[versus_select_result_order[0]] < 99) {
            versus_select_win_counts[versus_select_result_order[0]]++;
        }
    }
    ModeFrame = 0;
    SetMenuFrameOptions(0, 1, 0);
}
