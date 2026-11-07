#include "rr/menu_ui_bank.h"
void ovl662_func_80116808(void)
{
    CourseListIndex=D_800F4D99;
    {
        RrUiRow24 *ui=UiRows;
        s32 selected=CourseListIndex;
        ((RrMenuUiBank *)ui)->cell_view.cells0C[selected]=0;
    }
    if (D_800F3A42&0x2000) {
        if (CourseListIndex<3) {
            CourseListIndex++;
            MenuStepDirection=2;
            goto changed;
        }
    }
    if (D_800F3A42&0x8000) {
        if (CourseListIndex>0) {
            CourseListIndex--;
            MenuStepDirection=1;
            goto changed;
        }
    }
    MenuStepDirection=0;
    goto active;
changed:
    PlaySfx(0);
    if (CourseListIndex<0)CourseListIndex=0;
    else if (CourseListIndex>=4)CourseListIndex=3;
    D_800F4D99=(u8)CourseListIndex;
    GlyphRows[CourseListIndex].control02=-6;
active:
    UiRows->byte16=(u8)CourseListIndex+4;
    {
        RrUiRow24 *ui=UiRows;
        s32 selected=CourseListIndex;
        ((RrMenuUiBank *)ui)->cell_view.cells0C[selected]=2;
    }
}
