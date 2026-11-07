#include "types.h"
extern void ClearScreenOverlayState(void);
extern void InitBadgeRow(s32, s32, s32, s32, s32, s32, s32);
extern void SetMenuFrameOptions(s32, s32, s32);
extern u8 ScreenFadeState;
extern s16 *BadgeRows;
void options_init_brightness_test(void)
{
    ClearScreenOverlayState();
    ScreenFadeState = 2;
    InitBadgeRow(0, 12, 304, 296, 88, 0, 0);
    BadgeRows[1] = 1;
    SetMenuFrameOptions(1, 1, 0);
}
