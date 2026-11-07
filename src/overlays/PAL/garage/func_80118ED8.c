#include "rr/overlay662_screen_init.h"
void garage_func_80118ED8(void) {
    ModeFrame = 0;
    ScreenFrame = 0;
    garage_D_801272D4 = 0;
    MenuCursor = 0;
    MenuItemCount = 3;
    ResetScreenCounters();
    MenuArenas.second = CdDirectoryBuffer + 0x2D000;
    InitMenuArenas();
    LoadCursor = MenuArenas.second;
    InitUiRow(0, 24, 88, 68, 32, 0, 0, 1, 0, 83);
    InitUiRow(1, 24, 128, 68, 32, 0, 0, 1, 0, 42);
    InitUiRow(2, 24, 168, 68, 32, 0, 0, 1, 0, 85);
    InitPreviewImageRows();
    InitBadgeRow(1, 24, 232, 84, 34, 0, 0x501);
    SetupRenderer(5);
    ModeGroup = 1;
    ModeId = 68;
    LoadCursor = MenuArenas.first = MenuArenaBase;
    ModeSubState = 0;
}
