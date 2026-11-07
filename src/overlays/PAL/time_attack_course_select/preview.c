#include "rr/overlay660.h"

void time_attack_course_select_func_801163C4(void)
{
    RrRaceVehicle *state;
    RrProjectionScratchPrefix *scratch;

    ModeFrame = 0;
    ScreenFrame = 0;
    SetupRenderer(5);
    MenuCursor = 0;
    MenuItemCount = 3;
    ResetScreenCounters();
    InitPreviewImageRows();
    state = PlayerCars;
    scratch = RR_PROJECTION_SCRATCH;
    scratch->word18 = 120;
    scratch->word08 = -16;
    scratch->word0C = -248;
    scratch->word10 = -96;
    scratch->word1C = 0;
    scratch->word20 = 0;
    LoadCursor = MenuArenaBase;
    ResetCarState(state);
    state->pos20[1] = 10;
    state->pos20[2] = 308;
    state->rot54 = 2048;
    time_attack_course_select_preview_confirmation = 0;
    UnpackChoiceGlyph((s8 *)&SelectionRows, CourseChoiceTable[time_attack_course_select_preview_bank][time_attack_course_select_preview_selection].glyph);
    PlayerCourseChoices[0].first = CourseChoiceTable[time_attack_course_select_preview_bank][time_attack_course_select_preview_selection].first;
    PlayerCourseChoices[0].second = CourseChoiceTable[time_attack_course_select_preview_bank][time_attack_course_select_preview_selection].second;
    time_attack_course_select_available_car_counts[0] = CountChoiceGlyphs(&CourseChoiceTable[0][0]);
    time_attack_course_select_available_car_counts[1] = CountChoiceGlyphs(&CourseChoiceTable[1][0]);
}

void time_attack_course_select_func_80116530(void) {
    RrPreviewColor color;
    s32 drawFlags;
    s32 scale;
    s8 modelReady;
    s32 drawModel;
    s32 rowIndex;
    s32 first;
    s32 loadResult;
    s32 previousCar;
    s32 other;
    s32 selectionState;
    u8 confirmationState;
    u8 closingState;
    u8 headerState;
    s8 brightness;

    modelReady = 0;
    ModeFrame = (u32)ModeFrame + 1;
    PlayerCars[0].rot54 = ((u32)PlayerCars[0].rot54 + 8) & 0xFFF;
    SetupMenuCarCamera(1);
    SetupViewport(6);
    loadResult = LoadCarRangeStep(0, 1);
    if (loadResult != 1) {
        if (((s8 *)ImageRows)[1] == 1) {
            ((s8 *)ImageRows)[1] = 0;
        }
        goto draw_rows;
    }
    SetCarBounds(PlayerCars);
    if (((s8 *)ImageRows)[1] == 7) {
        ((s8 *)ImageRows)[1] = loadResult;
    }
    modelReady = 1;
    switch (ModeSubState) {
    case 7:
        if (UiRows[(s16)(time_attack_course_select_preview_bank ^ 1) * 3 + 4].byte14 == 5) {
            ModeSubState = 0;
        }
    case 0:
        SetMenuFrameOptions(1, 1, 1);
        previousCar = PackChoiceGlyph((s8 *)&SelectionRows);
        StepMenuValue(1, 2, (char *)&time_attack_course_select_preview_selection,
                      time_attack_course_select_available_car_counts[time_attack_course_select_preview_bank], 0);
        UnpackChoiceGlyph((s8 *)&SelectionRows,
                      CourseChoiceTable[time_attack_course_select_preview_bank][time_attack_course_select_preview_selection].glyph);
        if (((u32)previousCar << 16) != ((u32)PackChoiceGlyph((s8 *)&SelectionRows) << 16)) {
            ResetScreenCounters();
            LoadCursor = MenuArenaBase;
            UnpackChoiceGlyph((s8 *)&SelectionRows,
                          CourseChoiceTable[time_attack_course_select_preview_bank][time_attack_course_select_preview_selection].glyph);
            PlayerCourseChoices[0].first = CourseChoiceTable[time_attack_course_select_preview_bank][time_attack_course_select_preview_selection].first;
            PlayerCourseChoices[0].second = CourseChoiceTable[time_attack_course_select_preview_bank][time_attack_course_select_preview_selection].second;
            ((s8 *)ImageRows)[1] = 6;
        }
        UiRows[time_attack_course_select_preview_row_map[time_attack_course_select_preview_bank][MenuCursor]].cells[0] = 0;
        StepMenuValue(0, 2, (char *)&MenuCursor, MenuItemCount, 0);
        UiRows[time_attack_course_select_preview_row_map[time_attack_course_select_preview_bank][MenuCursor]].cells[0] = 2;
        selectionState = UiRows[time_attack_course_select_preview_bank * 3 + 4].byte14;
        if (selectionState != 2) {
            break;
        }
        if (PlayerControllers[0].a != 0) {
            UiRows[time_attack_course_select_preview_row_map[time_attack_course_select_preview_bank][MenuCursor]].cells[0] = 1;
            if (MenuCursor == 0) {
                PlaySfx(1);
                time_attack_course_select_preview_confirmation = selectionState;
                ModeSubState = 4;
            } else if (MenuCursor == 1 && CourseChoiceTable[1][0].glyph != -1) {
                PlaySfx(1);
                time_attack_course_select_preview_selection = 0;
                rowIndex = 0;
                do {
                    if (UiRows[time_attack_course_select_preview_bank * 3 + rowIndex + 4].byte14 == 2) {
                        UiRows[time_attack_course_select_preview_bank * 3 + rowIndex + 4].byte14 = 3;
                    }
                    UiRows[(s16)(time_attack_course_select_preview_bank ^ 1) * 3 + rowIndex + 4].byte14 = 0;
                    rowIndex += 1;
                } while (rowIndex < 3);
                ModeSubState = 7;
                MenuCursor = 0;
                time_attack_course_select_preview_bank ^= 1;
            } else if (MenuCursor == 2) {
                PlaySfx(1);
                rowIndex = 0;
                do {
                    if (UiRows[time_attack_course_select_preview_bank * 3 + rowIndex + 4].byte14 == 2) {
                        UiRows[time_attack_course_select_preview_bank * 3 + rowIndex + 4].byte14 = 3;
                    }
                    rowIndex += 1;
                } while (rowIndex < 3);
                goto close_selection;
            }
        } else if (PlayerControllers[0].b != 0) {
            PlaySfx(2);
            rowIndex = 0;
            do {
                if (UiRows[time_attack_course_select_preview_bank * 3 + rowIndex + 4].byte14 == 2) {
                    UiRows[time_attack_course_select_preview_bank * 3 + rowIndex + 4].byte14 = 3;
                }
                rowIndex += 1;
            } while (rowIndex < 3);
        close_selection:
            ((s8 *)ImageRows)[1] = 3;
            ModeSubState = 5;
            BadgeRows->state10 = 4;
        }
        break;
    case 3:
        MenuFadeTimer = (u16)MenuFadeTimer - 1;
        SetModelTexturePage(-1);
        AddScreenFadeTiles(&CurrentDrawBuf->ordering.ordering70[0], (0x40 - MenuFadeTimer) * 4, 0x45);
        if (MenuFadeTimer <= 0) {
            SeqMuteVolume();
            MenuScriptId = 0;
            ModeGroup = 1;
            ModeId = 0x23;
            ModeSubState = 0;
            goto save_preview_arena;
        }
        break;
    case 4:
        confirmationState = UiRows[10].byte14;
        if (confirmationState == 2) {
            UiRows[SelectionRows.selection.pad02[1] + 10].cells[0] = 0;
            StepMenuValue(0, 1, (char *)&SelectionRows.selection.pad02[1], 2, 0);
            UiRows[SelectionRows.selection.pad02[1] + 10].cells[0] = confirmationState;
            if (PlayerControllers[0].a != 0) {
                PlaySfx(1);
                SetMenuFrameOptions(0, 1, 1);
                UiRows[SelectionRows.selection.pad02[1] + 10].cells[0] = 1;
                UiRows[10].byte14 = 7;
                UiRows[11].byte14 = 7;
                ModeSubState = 3;
                MenuFadeTimer = 0x40;
                SeqFadeVolume(0, 0x3C);
            save_preview_arena:
                MenuArenas.first = LoadCursor;
            } else if (PlayerControllers[0].b != 0) {
                PlaySfx(2);
                UiRows[10].byte14 = 3;
                UiRows[11].byte14 = 3;
                ModeSubState = 6;
            }
        }
        break;
    case 6:
        if (UiRows[10].byte14 == 5) {
            ModeSubState = 0;
            time_attack_course_select_preview_confirmation = 1;
            UiRows[10].byte14 = 0;
            UiRows[11].byte14 = 0;
        }
        break;
    case 5:
        closingState = UiRows[time_attack_course_select_preview_bank * 3 + 4].byte14;
        if (closingState == 5 && ((s8 *)ImageRows)[1] == closingState) {
            ResetScreenCounters();
            time_attack_course_select_func_80114920();
            ModeGroup = 1;
            ModeId = 50;
            ModeSubState = 0;
            goto end;
        }
        break;
    }
draw_rows:
    if (time_attack_course_select_preview_confirmation == 0 && ((s8 *)ImageRows)[1] == 2) {
        rowIndex = 0;
        do {
            if (UiRows[rowIndex].byte14 == 9) {
                UiRows[rowIndex].byte14 = 3;
            }
            rowIndex += 1;
        } while (rowIndex < 4);
    }
    headerState = UiRows[0].byte14;
    if (headerState == 5) {
        if (time_attack_course_select_preview_confirmation == 0) {
            time_attack_course_select_preview_confirmation = 1;
            BadgeRows->half02 = 1;
        }
        if (ModeSubState == 7) {
            other = (s16)(time_attack_course_select_preview_bank ^ 1);
            if (UiRows[other * 3 + 4].byte14 == headerState) {
                first = time_attack_course_select_preview_bank * 3 + 4;
            } else {
                first = other * 3 + 4;
            }
        } else {
            first = time_attack_course_select_preview_bank * 3 + 4;
        }
        rowIndex = 0;
        do {
            DrawUiRow(first + rowIndex);
            rowIndex += 1;
        } while (rowIndex < 3);
        if (time_attack_course_select_preview_confirmation == 2) {
            DrawUiRow(10);
            DrawUiRow(11);
        }
    }
    drawModel = modelReady;
    if (((u32)((u8)((s8 *)ImageRows)[1] - 1) < 2U) && (drawModel == 1)) {
        DrawSelectionIconGrid(0);
        PrepareCarDrawPose(PlayerCars);
        scale = 0x100;
        if (((s8 *)ImageRows)[1] == drawModel) {
            brightness = ~((ImageRows[2] * 0xFF) / 30);
            color.channels.r = brightness;
            color.channels.g = brightness;
            color.channels.b = brightness;
            drawFlags = 0x102;
            scale = (ImageRows[2] << 8) / 30;
        } else {
            drawFlags = 1;
            color.channels.r = 0x80;
            color.channels.g = 0x80;
            color.channels.b = 0x80;
        }
        DrawPreviewCar(PlayerCars, color.argument, scale, drawFlags);
    }
    UpdateBadgeRowAnim(0);
    if (UpdateImageRow(0) > 0) {
        DrawMenuStepArrows(0xA0, 0x90, 8, 1, time_attack_course_select_preview_selection, 0,
                      time_attack_course_select_available_car_counts[time_attack_course_select_preview_bank]);
    }
end:
    UpdateScreenFade(time_attack_course_select_preview_bank != 0 ? 0x10036 : 0x10032);
}
