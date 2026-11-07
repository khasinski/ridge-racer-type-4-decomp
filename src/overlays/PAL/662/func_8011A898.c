#include "rr/diagnostics.h"
#include "rr/overlay662_garage.h"
#include "rr/menu_setup.h"
extern const char ovl662_D_801148F0[];
extern s16 ModeGroup,ModeId,D_800F4D8A;
void InitUiRow(s32,s32,s32,s32,s32,s32,s32,s32,s32,s32,...);
void ovl662_func_8011A898(void) {
    DebugPrintStub(ovl662_D_801148F0);
    ovl662_func_8011A788(0);
    ovl662_D_801276A8.dirtyD3 = 0;
    ClearScreenOverlayState();
    InitBadgeRow(0, 12, 304, 296, 88, 0, 0);
    InitUiRow(0, 140, 136, 28, 32, 0, 0, 1, 0, 104);
    InitUiRow(1, 140, 176, 28, 32, 0, 0, 1, 0, 105);
    ModeGroup = 3;
    ModeId = 4;
    D_800F4D8A = 0;
}
