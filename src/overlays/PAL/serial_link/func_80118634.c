#include "rr/menu_text_rows.h"
#include "rr/overlay_menu.h"
extern s32 ModeFrame;
extern s16 MenuCursor;
extern s16 MenuItemCount;
extern s32 serial_link_D_80118E2C[22];
extern s32 TotalRaceTime;
extern s16 NumCars;
extern s16 LinkResultPlayerByRank[8];
extern s16 PlayerWinCounts[8];
extern s16 FinalCarRanks[8];
extern s16 LinkState;
extern void SetMenuFrameOptions(s32, s32, s32);

void serial_link_func_80118634(void) {
    s32 i;

    /* The active participant count is0..8; order entries and the preexisting
     * inverse position for participant0 are valid slots0..7. */
    ModeFrame = 0;
    InitMenuTextRows(SharedWorkBuffer);
    MenuCursor = 0;
    MenuItemCount = 3;
    for (i = 0; i < 11; i++) {
        SetMenuTextRow(i, i, 0, 0, serial_link_D_80118E2C[i]);
    }
    for (i = 11; i < 22; i++) {
        SetMenuTextRow(i, i - 11, 0, 0, -1);
    }
    for (i = 33; i < 44; i++) {
        SetMenuTextRow(i, i - 33, 2, 0, serial_link_D_80118E2C[i - 22]);
    }
    if (TotalRaceTime >= 0) {
        for (i = 0; i < NumCars; i++) {
            LinkResultPlayerByRank[FinalCarRanks[i]] = i;
        }
        if (PlayerWinCounts[LinkResultPlayerByRank[0]] < 99) {
            PlayerWinCounts[LinkResultPlayerByRank[0]]++;
        }
    }
    LinkState = 0;
    SetMenuFrameOptions(0, 1, 0);
}
