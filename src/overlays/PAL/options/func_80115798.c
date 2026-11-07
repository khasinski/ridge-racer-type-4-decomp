#include "rr/options_calibration.h"

void options_init_screen_adjust(void)
{
    ClearScreenOverlayState();
    MenuCursor = 0;
    InitBadgeRow(0, 12, 304, 296, 88, 0, 0);
    BadgeRows[0].half02 = 1;
    SetMenuFrameOptions(1, 1, 0);
    DebugPrintStub(options_screen_offset_fmt, Options.display.x34, Options.display.y38);
}
