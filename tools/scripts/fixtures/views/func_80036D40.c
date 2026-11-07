#include "rr/menu_choice.h"

void UpdateCourseListChoice(void) {
    s32 state;
    u32 selected=D_800F4D99;
    CourseListIndex=selected;
    ((u8 *)UiRows)[12+selected]=0;
    if(D_800F3A42&0x2000) {
        if(CourseListIndex<3) {

            CourseListIndex=CourseListIndex+1;
            MenuStepDirection=2;
            goto changed;
        }
    }
    if(D_800F3A42&0x8000) {
        if(CourseListIndex>0) {
            CourseListIndex=CourseListIndex-1;
            MenuStepDirection=1;
            goto changed;
        }
    }
    MenuStepDirection=0;
    goto active;
changed:
    /* The sound helper can change selection and backing pointers. */
    PlaySfx(0);
    if(CourseListIndex<0)CourseListIndex=0;
    else if(CourseListIndex>=4)CourseListIndex=3;
    Dialogue[4]=(u8)CourseListIndex+34;
    D_800F4D99=CourseListIndex;
    GlyphRows[CourseListIndex].control02=-6;
    GlyphRows[CourseListIndex].flags00=0;
    UiRows->byte16=(u8)CourseListIndex+4;
active:
    {
        u8 *ui=(u8 *)UiRows;
        s32 active;

        active=CourseListIndex;
        *(u8 *)((unsigned long)ui+(unsigned long)active+12UL)=2;
    }
    if(MenuConfirmPressed) {
        state=BadgeRows->state10;
        if(state==3) {
            PlaySfx(1);
            {
                RrMenuSettings2C *settings=&SelectionState;
                RrBadgeRow1C *control=BadgeRows;
                RrUiRow24 *ui=UiRows;

                /* Four copies of the same byte-aligned source record. */
                settings->copies[0]=SelectionRows;
                settings->copies[1]=SelectionRows;
                settings->copies[2]=SelectionRows;
                settings->copies[3]=SelectionRows;
                control->state10=4;
                ui->byte14=state;
                {
                    u32 byte=SelectionRows.bytes[1];
                    s32 mode;

                    mode=2;
                    settings->mode1A=mode;
                    settings->copies[0].bytes[1]=byte;
                }
            }
            func_80039920();
            ResetScreenCounters();
            {
                s32 zero=0;
                void *resource;
                s32 one;
                s32 last;

                /* Both preceding helpers may replace the resource buffer. */
                resource=MenuArenaBase;

                LoadCursor=resource;
                one=1;

                last=zero;
                SetMenuFrameOptions(zero,one,last);
            }
        }
    }
}
