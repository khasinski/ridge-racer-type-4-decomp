#include "rr/overlay662_name_init.h"
void garage_func_80118930(void) {
    ModeFrame = 0;
    ScreenFrame = 0;
    garage_D_801273A2 = 0;
    garage_D_801273A3 = 0;
    garage_D_801273A0 = 0;
    garage_D_801273B4 = 0;
    CopyStringClearTail((char *)garage_D_801273A8, (char *)&SaveProgress);
    ClearScreenOverlayState();
    InitBadgeRow(0, 4, 172, 312, 216, 0, 0x806);
    BadgeRows[0].half02 = 1;
    InitBadgeRow(1, 208, 84, 108, 80, 0, 0x905);
    BadgeRows[1].half02 = 1;
    SetMenuFrameOptions(0, 1, 1);
    ModeGroup = 1;
    ModeId = 28;
    ModeSubState = 0;
}
