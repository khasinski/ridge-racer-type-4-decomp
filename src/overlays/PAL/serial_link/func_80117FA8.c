#include "rr/overlay667_course_menu.h"

void serial_link_func_80117FA8(void) {
    s32 previewIndex;
    s32 waitResult;
    s32 backIndex;
    s32 close;
    s32 row;
    s32 cellCount;
    u16 previousCourse;
    u8 activeState;
    s32 detailState;
    RrUiRow24 *preview;
    RrUiRow24 *back;

    if (ExitOnLinkLost() != 0) {
        SeqMuteVolume();
        return;
    }
    waitResult = UpdateLinkHandshake(2);
    ModeFrame = (s32)((u32)ModeFrame + 1U);
    ScreenFrame = (s32)((u32)ScreenFrame + 1U);
    if ((ModeSubState == 3) || (LoadTrackNodesStep() == 1)) {
        switch (ModeSubState) {                       /* irregular */
        case 0:
            SetMenuFrameOptions(1, 1, 1);
            previousCourse = CourseMenuSelection;
            StepMenuValue(3, 2, (char *)&CourseMenuSelection, AvailableCourseCount, 0);
            if ((s16) previousCourse != (s16) CourseMenuSelection) {
                ResetScreenCounters();
            } else {
                DrawCourseMapPreview((s16) previousCourse, 2);
            }
            CourseIndex = DecodeCourseIndex((s16) CourseMenuSelection);
            UiRows[MenuCursor+18].cells[0] = 0;
            StepMenuValue(0, 2, (char *)&MenuCursor, 3, 0);
            UiRows[MenuCursor+18].cells[0] = 2;
            row = 0;
            if (UiRows[21].byte02 != 0) {
                do {
                    if (row == (s16) CourseMenuSelection) {
                        UiRows[21].cells[row] = 2;
                        UiRows[21].byte16 = row;
                    } else {
                        UiRows[21].cells[row] = 0;
                    }
                    cellCount=UiRows[21].byte02;
                    row += 1;
                } while (row < cellCount);
            }
            activeState = UiRows[18].byte14;
            close = 0;
            if (activeState == 2) {
                if (PlayerControllers[0].a != 0) {
                    PlaySfx(1);
                    UiRows[MenuCursor+18].cells[0] = 1;
                    if ((s16) MenuCursor == 0) {
                        row = 0;
                        previewIndex = 0;
                        do {
                            preview = (RrUiRow24 *)((unsigned long)(previewIndex*sizeof(RrUiRow24))+(unsigned long)UiRows+0x288);
                            if (preview->byte14 == 2) {
                                preview->byte14 = 7U;
                            }
                            row += 1;
                            previewIndex++;
                        } while (row < 4);
                        ResetScreenCounters();
                        MenuFadeTimer = 0x40;
                        LoadCursor = MenuArenas.first;
                        SeqFadeVolume(0, 0x3C);
                        BeginLinkSync();
                        ModeSubState = 3;
                        SetMenuFrameOptions(0, 1, 1);
                    } else if ((s16) MenuCursor == 1) {
                        UiRows[23].byte14 = 0U;
                        ModeSubState = 4;
                    } else if ((s16) MenuCursor == activeState) {
                        close = 1;
                    }
                } else if (PlayerControllers[0].b != 0) {
                    PlaySfx(2);
                    close = 1;
                }
                if (close == 1) {
                    SetMenuFrameOptions(0, 1, 1);
                    row = 0;
                    backIndex = 0;
                    do {
                        back = (RrUiRow24 *)((unsigned long)(backIndex*sizeof(RrUiRow24))+(unsigned long)UiRows+0x288);
                        if (back->byte14 == 2) {
                            back->byte14 = 3U;
                        }
                        row += 1;
                        backIndex++;
                    } while (row < 5);
                    ModeSubState = 8;
                }
            }
            break;
        case 3:
            DrawCourseMapPreview((s16) CourseMenuSelection, 2);
            if ((waitResult == 0) && (MenuFadeTimer == 0x40)) {
                DrawUiRow(0x18);
            } else {
                CourseMenuPhase = 1;
                if (MenuFadeTimer > 0) {
                    MenuFadeTimer = (s16)((u16) MenuFadeTimer - 1);
                }
                AddScreenFadeTiles(((RrFrameOrderingPrefix *)(void *)CurrentDrawBuf)->ordering70, (0x40 - MenuFadeTimer) * 4, 0x45);
                if (MenuFadeTimer <= 0) {
                    FinalCarRanks = -1;
                    SeqMuteVolume();
                    ModeGroup = 1;
                    ModeId = 0x29;
                    ModeSubState = 0;
                    LinkState = 0;
                    _sio_control(3, 0, 0);
                }
            }
            break;
        case 4:
            detailState = UiRows[23].byte14;
            if (detailState == 2) {
                SetMenuFrameOptions(1, 1, 0);
                UiRows[23].cells[0] = detailState;
                StepMenuValue(1, 4, (char *)&NumLaps, 0xA, (s32) detailState);
                if (MenuConfirmPressed != 0) {
                    PlaySfx(1);
                    UiRows[23].cells[0] = 1U;
                    UiRows[23].byte14 = 3U;
                }
            } else if (detailState == 5) {
                ModeSubState = 0;
            }
            DrawCourseMapPreview((s16) CourseMenuSelection, 2);
            break;
        case 8:
            if (UiRows[18].byte14 == 5) {
                serial_link_func_801161A8((1 - LinkRoleSlot) * 2);
                ModeGroup = 1;
                ModeId = 0x3C;
                ModeSubState = 0;
            }
            break;
        }
    }
    DrawCourseInfoPanel((s16) CourseMenuSelection, 2);
    row = 0;
    do {
        DrawUiRow(row + 0x12);
        row += 1;
    } while (row < 4);
    if (UiRows[18].byte14 == 2) {
        if (ModeSubState != 4) {
            DrawMenuStepArrows(0xCE, 0xD0, 0x5E, 0, (s32) (s16) CourseMenuSelection, -1, (s32)((u32)AvailableCourseCount + 1U));
            goto block_63;
        }
        goto block_64;
    }
block_63:
    if (ModeSubState == 4) {
block_64:
        DrawUiRow(0x17);
    }
    MenuActivePlayerPlusOne = 0;
    UpdateScreenFade(0x10026);
}
