#include "rr/diagnostics.h"
#include "rr/overlay662_garage.h"
#include "rr/menu_setup.h"
extern const char garage_enter_load_back_message[];
extern s16 ModeGroup,ModeId,ModeSubState;
void InitUiRow(s32,s32,s32,s32,s32,s32,s32,s32,s32,s32,...);
void garage_enter_editor_full_reset(void) {
    DebugPrintStub(garage_enter_load_back_message);
    garage_reset_editor_state(1);
    garage_init_option_tables(-1);
    garage_save_undo_copy();
    garage_logo_state.dirtyD3 = 0;
    ClearScreenOverlayState();
    InitBadgeRow(0, 12, 304, 296, 88, 0, 0);
    InitUiRow(0, 140, 136, 28, 32, 0, 0, 1, 0, 104);
    InitUiRow(1, 140, 176, 28, 32, 0, 0, 1, 0, 105);
    ModeGroup = 3;
    ModeId = 4;
    ModeSubState = 0;
}
