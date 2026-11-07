#include "rr/menu_ui_bank.h"
void ovl662_func_80116690(void)
{
    SelectedTeamIndex=SelectionRows.bytes[0];
    {
        RrUiRow24 *ui=UiRows;
        s32 selected=SelectedTeamIndex;
        ((RrMenuUiBank *)ui)->cell_view.cells0C[selected]=0;
    }
    if (Pad0RepeatButtons&0x2000) {
        if (SelectedTeamIndex<3) {
            SelectedTeamIndex++;
            MenuStepDirection=2;
            goto changed;
        }
    }
    if (Pad0RepeatButtons&0x8000) {
        if (SelectedTeamIndex>0) {
            SelectedTeamIndex--;
            MenuStepDirection=1;
            goto changed;
        }
    }
    MenuStepDirection=0;
    goto active;
changed:
    PlaySfx(0);
    if (SelectedTeamIndex<0)SelectedTeamIndex=0;
    else if (SelectedTeamIndex>=4)SelectedTeamIndex=3;
    SelectionRows.bytes[0]=(u8)SelectedTeamIndex;
    GlyphRows[SelectedTeamIndex].control02=-6;
    GlyphRows[SelectedTeamIndex].flags00=0;
active:
    UiRows->byte16=(u8)SelectedTeamIndex;
    {
        RrUiRow24 *ui=UiRows;
        s32 selected=SelectedTeamIndex;
        ((RrMenuUiBank *)ui)->cell_view.cells0C[selected]=2;
    }
}
