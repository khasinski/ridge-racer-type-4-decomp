#include "rr/overlay_menu_init.h"

void options_func_80114FD8(void)
{
    ClearScreenOverlayState();
    InitUiRow(0,12,72,68,32,0,0,1,0,86);
    InitUiRow(1,12,112,68,32,0,0,1,0,87);
    InitUiRow(2,12,152,68,32,0,0,1,0,88);
    InitUiRow(3,12,192,68,32,0,0,1,0,85);
    InitUiRow(4,100,72,98,32,0,1,1,10,89);
    InitUiRow(5,100,112,98,32,0,1,1,11,89);
    InitUiRow(6,100,152,68,32,0,1,1,0,91);
    InitUiRow(7,100,192,68,32,0,1,1,0,92);
    SetMenuFrameOptions(1,1,1);
}
