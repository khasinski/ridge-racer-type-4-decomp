#include "rr/garage_name_init.h"
void garage_init_name_entry(void) {
    ModeFrame = 0;
    ScreenFrame = 0;
    garage_name_key_column = 0;
    garage_name_key_row = 0;
    garage_name_cursor = 0;
    garage_name_key_index = 0;
    CopyStringClearTail((char *)garage_name_buffer, (char *)&SaveProgress);
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
