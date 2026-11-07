#include "rr/overlay663.h"

void ovl663_update_selection(void) {
    s32 car;
    s32 renderMask;
    s32 modelReady;
    s16 fadeTimer;
    s32 mirrorSet;
    s32 i;
    s32 glyphIndex;

    ModeFrame = (u32)ModeFrame + 1;
    ScreenFrame = (u32)ScreenFrame + 1;
    switch ((u16)ModeSubState) {
    case 0:
        i = 0;
        if (BadgeRows[1].state10 == 3) {
            ModeSubState = 1;
            SetMenuFrameOptions(1, 1, 1);
            i = 0;
        }
        break;
    case 1:
        DrawMenuStepArrows(0xA0, 0x108, 0x76, 0, (s32)CourseMenuSelection, 0, ovl663_course_count);
        StepMenuValue(1, 2, (char *)&CourseMenuSelection, ovl663_course_count, 0);
        i = 0;
        CourseIndex = DecodeCourseIndex(CourseMenuSelection);
        while (i < (s32)UiRows[2].byte02) {
            if (i == ((u16)CourseMenuSelection & 7)) {
                UiRows[2].cells[i] = (s8)((u8)ReverseCourse + 2);
                UiRows[2].byte16 = i;
            } else if (i >= ovl663_course_count) {
                UiRows[2].cells[i] = 1;
            } else {
                UiRows[2].cells[i] = (u8)ReverseCourse;
            }
            i += 1;
        }
        UiRows[MenuCursor].cells[0] = 0;
        StepMenuValue(0, 2, (char *)&MenuCursor, 2, 0);
        i = 0;
        UiRows[MenuCursor].cells[0] = 2;
        ovl663_selected_record = -1;
        if (PlayerControllers[0].a != 0) {
            PlaySfx(1);
            UiRows[MenuCursor].cells[0] = 1;
            if (MenuCursor == 0) {
                ovl663_selected_record = 0;
                ModeSubState = 2;
            } else if (MenuCursor == 1) {
                i = 1;
            }
        } else if (PlayerControllers[0].b != 0) {
            PlaySfx(2);
            i = 1;
        }
        if (i == 1) {
            SetMenuFrameOptions(0, 1, 1);
            i = 0;
            MenuFadeTimer = 0x40;
            ModeSubState = 8;
            do {
                UiRows[i].byte14 = 3;
                i += 1;
            } while (i < 3);
            BadgeRows[1].state10 = 4;
        }
        break;
    case 2:
        StepMenuValue(0, 4, (char *)&ovl663_selected_record, 5, 0);
        if (PlayerControllers[0].b != 0) {
            PlaySfx(2);
            ModeSubState = 1;
        } else if (PlayerControllers[0].a != 0) {
            SetMenuFrameOptions(0, 1, 1);
            ModeSubState = 3;
            ResetScreenCounters();
            LoadCursor = MenuArenaBase;
            mirrorSet = CourseMenuSelection >= 8;
            if (ovl663_record_kind == 1) {
                car = LapRecords[mirrorSet][CourseIndex][ovl663_selected_record].car;
            } else {
                car = RaceRecords[mirrorSet][CourseIndex][ovl663_selected_record].car;
            }
            UnpackChoiceGlyph((s8 *)&SelectionRows, car);
            if (SelectionRows.choice.bytes[2] == 0x27) {
                if (SelectionRows.choice.bytes[0] != 2) {
                    glyphIndex = SelectionRows.choice.bytes[1] + (SelectionRows.choice.bytes[0] * 4);
                } else {
                    glyphIndex = 0;
                }
                PlayerCourseChoices[0].first = (s16)(s8)CourseChoiceTable[0][glyphIndex].first;
                PlayerCourseChoices[0].second = 0;
            } else {
                PlayerCourseChoices[0].first = 0;
                PlayerCourseChoices[0].second = 0;
            }
            i = 0;
            do {
                UiRows[i].byte14 = 3;
                i += 1;
            } while (i < 3);
            BadgeRows[1].state10 = 4;
        }
        break;
    case 3:
        modelReady = LoadCarRangeStep(0, 1);
        if ((modelReady == 1) && (UiRows->byte14 == 5)) {
            ovl663_init_result_preview();
            ModeGroup = modelReady;
            ModeId = 0x4D;
            ModeSubState = 0;
        }
        break;
    case 8:
        fadeTimer = (u16)MenuFadeTimer - 1;
        MenuFadeTimer = (u16)fadeTimer;
        AddScreenFadeTiles(&CurrentDrawBuf->ordering.ordering70[0], (0x40 - fadeTimer) * 4, 0x45);
        i = 0;
        if ((s16)MenuFadeTimer <= 0) {
            ovl663_init_menu();
            ModeGroup = 1;
            ModeId = 0x4A;
            ModeSubState = 0;
            return;
        }
        break;
    default:
        break;
    }
    i = 0;
    do {
        DrawUiRow(i);
        i += 1;
    } while (i < 3);
    UpdateBadgeRowAnim(1);
    renderMask = 0x10063;
    if (ovl663_record_kind == 1) {
        renderMask = 0x1005F;
    }
    UpdateScreenFade(renderMask);
}
