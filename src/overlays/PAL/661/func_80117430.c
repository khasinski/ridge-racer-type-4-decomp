#include "rr/overlay661_course_menu.h"

void ovl661_func_80117430(void) {
    s32 var_a0_3;
    s32 var_s2;
    s32 var_s0;
    u16 temp_s0;
    u8 *temp_v1_2;
    u8 temp_a2;
    s32 temp_s0_2;
    s32 temp_s0_3;
    s32 temp_s1;
    RrUiRow24 *temp_v1;
    RrUiRow24 *temp_v1_3;

    ModeFrame=(u32)ModeFrame+1;
    ScreenFrame=(u32)ScreenFrame+1;
    if ((ModeSubState == 3) || (LoadTrackNodesStep() == 1)) {
        switch (ModeSubState) {
        case 0:
            SetMenuFrameOptions(1, 1, 1);
            temp_s0 = CourseMenuSelection;
            StepMenuValue(3, 2, (char *)&CourseMenuSelection, ovl661_D_80118414, 0);
            if ((s16) temp_s0 != (s16) CourseMenuSelection) {
                ResetScreenCounters();
            } else {
                DrawCourseMapPreview((s16) temp_s0, 1);
            }
            CourseIndex = DecodeCourseIndex((s16) CourseMenuSelection);
            UiRows[18+MenuCursor].cells[0] = 0;
            StepMenuValue(0, 2, (char *)&MenuCursor, MenuItemCount, 0);
            UiRows[18+MenuCursor].cells[0] = 2;
            var_s0 = 0;
            if (UiRows[22].byte02 != 0) {
                do {
                    if (var_s0 == (s16) CourseMenuSelection) {
                        *((u8 *)UiRows+var_s0+0x324) = 2;
                        UiRows[22].byte16 = var_s0;
                    } else {
                        *((u8 *)UiRows+var_s0+0x324) = 0;
                    }
                    var_s0 += 1;
                } while (var_s0 < (s32) UiRows[22].byte02);
            }
            temp_s0_2 = UiRows[18].byte14;
            var_s2 = 0;
            if (temp_s0_2 == 2) {
                if (PlayerControllers[0].a != 0) {
                    PlaySfx(1);
                    UiRows[18+MenuCursor].cells[0] = 1;
                    if ((s16) MenuCursor == 0) {
                        var_s0 = 0;
                        do {
                            temp_v1 = (RrUiRow24 *)((unsigned long)(var_s0*36)+(unsigned long)UiRows+648);
                            if (temp_v1->byte14 == 2) {
                                temp_v1->byte14 = 7U;
                            }
                            var_s0 += 1;
                        } while (var_s0 < 4);
                        {
                            u8 *parameterTable = D_80118374;
                            RrSelectionRows *choice = &SelectionRows;
                            u8 first = choice->primary;
                            u8 second = choice->variant;
                            u8 third = choice->rows[0].selection;
                            choice->rows[1].selection = first;
                            choice->rows[1].variant = second;
                        temp_v1_2 = parameterTable + (s16)SelectedCarIndex;
                        temp_a2 = *temp_v1_2;
                        choice->rows[2].selection=third;
                        choice->rows[2].variant = (u8) choice->rows[0].variant;
                        choice->rows[1].parameter = temp_a2;
                        choice->rows[2].parameter = (u8) *temp_v1_2;
                        }
                        ResetScreenCounters();
                        MenuFadeTimer = 0x40;
                        LoadCursor = MenuArenas.first;
                        SeqFadeVolume(0, 0x3C);
                        ModeSubState = 3;
                        SetMenuFrameOptions(0, 1, 1);
                    } else if ((s16) MenuCursor == 1) {
                        UiRows[23].byte14 = 0U;
                        ModeSubState = 4;
                    } else if ((s16) MenuCursor == temp_s0_2) {
                        UiRows[24].byte14 = 0U;
                        UiRows[25].byte14 = 0;
                        MenuCursor = 0;
                        ModeSubState = 5;
                    } else if ((s16) MenuCursor == 3) {
                        var_s2 = 1;
                    }
                } else if (PlayerControllers[0].b != 0) {
                    PlaySfx(2);
                    var_s2 = 1;
                }
                if (var_s2 == 1) {
                    SetMenuFrameOptions(0, 1, 1);
                    SetModelTexturePage(-1);
                    var_s0 = 0;
                    do {
                        temp_v1_3 = (RrUiRow24 *)((unsigned long)(var_s0*36)+(unsigned long)UiRows+648);
                        if (temp_v1_3->byte14 == 2) {
                            temp_v1_3->byte14 = 3U;
                        }
                        var_s0 += 1;
                    } while (var_s0 < 5);
                    ModeSubState = 8;
                }
            }
        default:
            goto block_63;
        case 3:
            DrawCourseMapPreview((s16) CourseMenuSelection, 1);
            if (MenuFadeTimer > 0) {
                MenuFadeTimer = (u16) MenuFadeTimer - 1;
            }
            AddScreenFadeTiles(&CurrentDrawBuf->head70, (0x40 - MenuFadeTimer) * 4, 0x45);
            if (NumCars.halves[0] >= 3) {
                RaceType = 3;
                if (LoadCarRangeStep(2, NumCars.halves[0] - 2) == 1) {
                    MenuArenas.first = LoadCursor;
                    goto block_42;
                }
            } else {
                RaceType = 2;
block_42:
                if (MenuFadeTimer <= 0) {
                    SeqMuteVolume();
                    ovl661_D_80118448[0] = -1;
                    MenuScriptId = 0;
                    ModeGroup = 1;
                    ModeId = 0x23;
                    ModeSubState = 0;
                }
            }
            goto block_63;
        case 4:
            temp_s0_3 = UiRows[23].byte14;
            if (temp_s0_3 == 2) {
                SetMenuFrameOptions(1, 1, 0);
                UiRows[23].cells[0] = temp_s0_3;
                StepMenuValue(1, 4, (char *)&NumLaps, 0xA, (s32) temp_s0_3);
                if (PlayerControllers[0].a != 0) {
                    PlaySfx(1);
                    UiRows[23].cells[0] = 1U;
                    UiRows[23].byte14 = 3U;
                }
            } else {
                if (temp_s0_3 !=5) goto block_59;
                goto block_57;
            }
            goto block_59;
        case 5:
            temp_s1 = UiRows[24].byte14;
            if (temp_s1 == 2) {
                SetMenuFrameOptions(1, 1, 0);
                UiRows[24+MenuCursor].cells[0] = 0;
                StepMenuValue(0, 2, (char *)&MenuCursor, 2, 0);
                UiRows[24+MenuCursor].cells[0] = temp_s1;
                if ((s16)MenuCursor == 0) {
                    StepMenuValue(1,4,(char *)&NumCars.word,5,temp_s1);
                } else {
                    StepMenuValue(1,2,(char *)&SelectedCarIndex,4,0);
                }
                if (PlayerControllers[0].a != 0) {
                    PlaySfx(1);
                    UiRows[24].cells[0] = 1;
                    UiRows[24].byte14 = 3U;
                    UiRows[25].cells[0] = 1;
                    UiRows[25].byte14 = 3;
                    MenuCursor = 2;
                }
            } else if (temp_s1 == 5) {
                goto block_57;
            }
            goto block_59;
block_57:
            ModeSubState=0;
block_59:
            DrawCourseMapPreview((s16) CourseMenuSelection, 1);
            goto block_63;
        case 8:
            if (UiRows[18].byte14 == 5) {
                ovl661_func_80115598(0);
                ModeGroup = 1;
                ModeId = 0x38;
                ModeSubState = 0;
                goto block_64;
            }
            var_s0=0;
            break;
        }
    } else {
block_63:
        DrawCourseInfoPanel((s16) CourseMenuSelection, 1);
block_64:
        var_s0 = 0;
    }
    do {
        DrawUiRow(var_s0 + 0x12);
        var_s0 += 1;
    } while (var_s0 < 5);
    if ((UiRows[18].byte14 == 2) && ((u32) (ModeSubState - 4) >= 2U)) {
        DrawMenuStepArrows(0xCE, 0xD0, 0x5E, 0, (s32) (s16) CourseMenuSelection, -1, ovl661_D_80118414 + 1);
    }
    if (ModeSubState == 4) {
        var_a0_3 = 0x17;
        goto block_73;
    }
    if (ModeSubState == 5) {
        DrawUiRow(0x18);
        var_a0_3 = 0x19;
block_73:
        DrawUiRow(var_a0_3);
    }
    MenuActivePlayerPlusOne = 0;
    UpdateScreenFade(0x10026);
}
