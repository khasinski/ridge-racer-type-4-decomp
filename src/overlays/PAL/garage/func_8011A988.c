#include "rr/diagnostics.h"
#include "rr/overlay662_garage.h"
#include "rr/menu_setup.h"
extern const char garage_D_80114900[];
extern s16 ModeGroup,ModeId,ModeSubState;
void InitUiRow(s32,s32,s32,s32,s32,s32,s32,s32,s32,s32,...);
void garage_func_8011A988(void) {
    DebugPrintStub(garage_D_80114900);
    garage_func_8011A788(1);
    garage_func_8011D278(-1);
    garage_func_8011D120();
    garage_D_801276A8.dirtyD3 = 0;
    ClearScreenOverlayState();
    InitBadgeRow(0, 12, 304, 296, 88, 0, 0);
    InitUiRow(0, 140, 136, 28, 32, 0, 0, 1, 0, 104);
    InitUiRow(1, 140, 176, 28, 32, 0, 0, 1, 0, 105);
    ModeGroup = 3;
    ModeId = 4;
    ModeSubState = 0;
}
