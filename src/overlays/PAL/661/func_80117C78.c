#include "rr/menu_text_rows.h"
#include "rr/overlay_menu.h"

extern s16 MenuCursor;
extern s16 MenuItemCount;
extern s32 ovl661_D_80118378[22];
extern s32 TotalRaceTime;
extern s16 ovl661_D_80118448[2];
extern s16 ovl661_D_80118410[2];
extern s16 ovl661_D_80118418[2];
extern s32 ModeFrame;

extern void SetMenuFrameOptions(s32, s32, s32);

void ovl661_func_80117C78(void) {
    s32 i;

    InitMenuTextRows(SharedWorkBuffer);
    MenuCursor = 0;
    MenuItemCount = 3;
    for (i = 0; i < 11; i++) {
        SetMenuTextRow(i, i, 0, 0, ovl661_D_80118378[i]);
    }
    for (i = 11; i < 22; i++) {
        SetMenuTextRow(i, i - 11, 0, 0, -1);
    }
    for (i = 33; i < 44; i++) {
        SetMenuTextRow(i, i - 33, 2, 0, ovl661_D_80118378[i - 22]);
    }
    if (TotalRaceTime >= 0) {
        if (ovl661_D_80118448[0] > ovl661_D_80118448[1]) {
            ovl661_D_80118410[0] = 1;
            ovl661_D_80118410[1] = 0;
        } else {
            ovl661_D_80118410[0] = 0;
            ovl661_D_80118410[1] = 1;
        }
        if (ovl661_D_80118418[ovl661_D_80118410[0]] < 99) {
            ovl661_D_80118418[ovl661_D_80118410[0]]++;
        }
    }
    ModeFrame = 0;
    SetMenuFrameOptions(0, 1, 0);
}
