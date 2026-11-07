#include "types.h"
#include "scratchpad.h"
#include "rr/course_choice_types.h"
#include "rr/menu_choice.h"
#include "m2c_macros.h"
#include "game/car/aipath.h"
#include "game/menu/title.h"

extern int DrawTrackScenery();
extern int D_800AC308;
extern int ViewUsesCarTrackNode;

void DrawTrackSceneryForPlayer(int a0);
void UnpackChoiceGlyph(signed char *a0, int a1);
s32 PackChoiceGlyph(void *arg0);
s32 CountChoiceGlyphs(const RrCourseChoice4 *a0);

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

void DrawTrackSceneryForPlayer(int a0) {
    int v;
    if (ViewUsesCarTrackNode == 1) {
        if (a0 < 2) {
            v = D_800ABFE8;
        } else {
            v = D_800AC308;
        }
    } else {
        int *p = (int *)SCRATCHPAD;
        v = FindTrackNodeAtXZ(p[2], p[4]);
    }
    DrawTrackScenery(v);
}

void UnpackChoiceGlyph(signed char *a0, int a1) {
    int t = a1 / 160;
    a0[0] = t;
    a0[1] = a1 / 40 - t * 4;
    a0[2] = a1 % 40;
}

s32 PackChoiceGlyph(void *arg0) {
    return (M2C_FIELD(arg0, u8 *, 0) * 0xA0) + (M2C_FIELD(arg0, u8 *, 1) * 0x28) + M2C_FIELD(arg0, u8 *, 2);
}

s32 CountChoiceGlyphs(const RrCourseChoice4 *a0) {
    int i;
    for (i = 0; i < 9; i++) {
        if (a0[i].glyph == -1) break;
        if (i == 8 && a0[8].glyph != 0x167) break;
    }
    return i;
}
