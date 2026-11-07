#include "rr/overlay666.h"

void ovl666_init_preview(void) {
    RrProjectionScratchPrefix *sp;

    ModeFrame = 0;
    ScreenFrame = 0;
    ovl666_selected_course.word = 0;
    ovl666_preview_phase = 0;
    MenuCursor = 0;
    MenuItemCount = 2;
    InitUiRow(0, 24, 88, 68, 32, 0, 0, 1, 0, 84);
    InitUiRow(1, 24, 128, 68, 32, 0, 0, 1, 0, 85);
    SetupRenderer(5);
    ResetScreenCounters();
    InitPreviewImageRows();
    LoadCursor = MenuArenaBase;
    BadgeRows[0].half02 = 1;
    sp = RR_PROJECTION_SCRATCH;
    sp->word18 = 120;
    sp->word08 = -16;
    sp->word0C = -248;
    sp->word1C = 0;
    sp->word20 = 0;
    sp->word10 = -96;
    ResetCarState(PlayerCars);
    PlayerCars[0].pos20[1] = 10;
    PlayerCars[0].pos20[2] = 308;
    PlayerCars[0].rot54 = 2048;
    UnpackChoiceGlyph((s8 *)&SelectionRows,
                  ovl666_available_courses[CourseListIndex][ovl666_selected_course.word].glyph);
    PlayerCourseChoices[0].first = ovl666_available_courses[CourseListIndex][ovl666_selected_course.word].first;
    PlayerCourseChoices[0].second = ovl666_available_courses[CourseListIndex][ovl666_selected_course.word].second;
    SetMenuFrameOptions(0, 1, 1);
}

void ovl666_update_preview(void) {
    RrPreviewColor color;
    s32 drawFlags;
    s32 scale;
    s8 modelReady;
    s32 rowIndex;
    s32 drawModel;
    s32 loadResult;
    s32 previousCar;
    u8 selectionState;
    u8 confirmationState;
    u8 closingState;
    u16 course;
    s8 brightness;

    ModeFrame = (u32)ModeFrame + 1;
    modelReady = 0;
    ScreenFrame = (u32)ScreenFrame + 1;
    PlayerCars[0].rot54 = ((u32)PlayerCars[0].rot54 + 8) & 0xFFF;
    SetupMenuCarCamera(1);
    SetupViewport(6);
    if (ModeSubState != 3) {
        loadResult = LoadCarRangeStep(0, 1);
        if (loadResult != 1) {
            if (((s8 *)ImageRows)[1] == 1) {
                ((s8 *)ImageRows)[1] = 0;
            }
            rowIndex = 0;
            goto draw_selection_rows;
        }
        SetCarBounds(PlayerCars);
        if (((s8 *)ImageRows)[1] == 7) {
            ((s8 *)ImageRows)[1] = loadResult;
        }
    }
    modelReady = 1;
    switch (ModeSubState) {
    case 0:
        SetMenuFrameOptions(1, 1, 1);
        previousCar = PackChoiceGlyph((s8 *)&SelectionRows);
        StepMenuValue(1, 2, (char *)&ovl666_selected_course.halves[0],
                      ovl666_course_counts[CourseListIndex], 0);
        UnpackChoiceGlyph((s8 *)&SelectionRows,
                      ovl666_available_courses[CourseListIndex][ovl666_selected_course.word].glyph);
        if (((u32)previousCar << 16) != ((u32)PackChoiceGlyph((s8 *)&SelectionRows) << 16)) {
            ResetScreenCounters();
            LoadCursor = MenuArenaBase;
            UnpackChoiceGlyph((s8 *)&SelectionRows,
                          ovl666_available_courses[CourseListIndex][ovl666_selected_course.word].glyph);
            PlayerCourseChoices[0].first =
                ovl666_available_courses[CourseListIndex][ovl666_selected_course.word].first;
            PlayerCourseChoices[0].second =
                ovl666_available_courses[CourseListIndex][ovl666_selected_course.word].second;
            ((s8 *)ImageRows)[1] = 6;
        }
        UiRows[MenuCursor + 3].cells[0] = 0;
        StepMenuValue(0, 2, (char *)&MenuCursor, 2, 0);
        UiRows[MenuCursor + 3].cells[0] = 2;
        selectionState = UiRows[MenuCursor + 3].byte14;
        if (selectionState == 2) {
            if (PlayerControllers[0].a != 0) {
                UiRows[MenuCursor + 3].cells[0] = 1;
                if (MenuCursor == 0) {
                    PlaySfx(1);
                    ovl666_preview_phase = selectionState;
                    ModeSubState = 4;
                    goto draw_selection;
                } else if (MenuCursor == 1) {
                    PlaySfx(1);
                    rowIndex = 0;

                    do {
                        if (UiRows[rowIndex + 3].byte14 == 2) {
                            UiRows[rowIndex + 3].byte14 = 3;
                        }
                        rowIndex += 1;
                    } while (rowIndex < 2);
                    goto close_selection;
                }
            } else if (PlayerControllers[0].b != 0) {
                PlaySfx(2);
                rowIndex = 0;

                do {
                    if (UiRows[rowIndex + 3].byte14 == 2) {
                        UiRows[rowIndex + 3].byte14 = 3;
                    }
                    rowIndex += 1;
                } while (rowIndex < 2);
            close_selection:
                ((s8 *)ImageRows)[1] = 3;
                ModeSubState = 5;
                BadgeRows->state10 = 4;
            }
        }
        goto draw_selection;
    case 3:
        SetMenuFrameOptions(0, 1, 1);
        if (MenuFadeTimer > 0) {
            MenuFadeTimer = (u16)MenuFadeTimer - 1;
        }
        SetModelTexturePage(-1);
        AddScreenFadeTiles(&CurrentDrawBuf->ordering.ordering70[0], (0x40 - MenuFadeTimer) * 4, 0x45);
        loadResult = LoadCarRangeStep(1, 1);
        if ((loadResult == 1) && (MenuFadeTimer <= 0)) {
            RaceType = 6;
            course =
                ovl666_course_dispatch[SelectionRows.selection.primary][SelectionRows.selection.variant];
            ovl666_result_kind = loadResult;
            CourseMenuSelection = course;
            CourseIndex = DecodeCourseIndex((s16)course);
            SeqMuteVolume();
            MenuScriptId = 0;
            ModeGroup = loadResult;
            ModeId = 0x23;
            ModeSubState = 0;
            goto save_preview_arena;
        }
        goto draw_selection;
    case 4:
        confirmationState = UiRows[5].byte14;
        if (confirmationState == 2) {
            UiRows[SelectionRows.selection.pad02[1] + 5].cells[0] = 0;
            StepMenuValue(0, 1, (char *)&SelectionRows.selection.pad02[1], 2, 0);
            UiRows[SelectionRows.selection.pad02[1] + 5].cells[0] = confirmationState;
            if (PlayerControllers[0].a != 0) {
                PlaySfx(1);
                SetMenuFrameOptions(0, 1, 1);
                UiRows[SelectionRows.selection.pad02[1] + 5].cells[0] = 1;
                UiRows[5].byte14 = 7;
                UiRows[6].byte14 = 7;
                SelectionRows.selection.rows[0] = SelectionRows.entry;
                SelectionRows.selection.rows[0].parameter = 0x13;
                CourseIndex = 0;
                ModeSubState = 3;
                ResetScreenCounters();
                MenuFadeTimer = 0x40;
                SeqFadeVolume(0, 0x3C);
            save_preview_arena:
                MenuArenas.first = LoadCursor;
                goto draw_selection;
            }
            if (PlayerControllers[0].b != 0) {
                PlaySfx(2);
                UiRows[5].byte14 = 3;
                UiRows[6].byte14 = 3;
                ModeSubState = 6;
            }
        }
        goto draw_selection;
    case 6:
        if (UiRows[5].byte14 == 5) {
            ModeSubState = 0;
            ovl666_preview_phase = 1;
            UiRows[5].byte14 = 0;
            UiRows[6].byte14 = 0;
        }
        goto draw_selection;
    case 5:
        SetMenuFrameOptions(0, 1, 1);
        closingState = UiRows[3].byte14;
        if ((closingState == 5) && (((s8 *)ImageRows)[1] == closingState)) {
            ResetScreenCounters();
            ovl666_init_menu();
            ModeGroup = 1;
            ModeId = 0x4F;
            ModeSubState = 0;
            goto end;
        }
        goto draw_selection;
    }
draw_selection:
    rowIndex = 0;
draw_selection_rows:
    do {
        DrawUiRow(rowIndex + 3);
        rowIndex += 1;
    } while (rowIndex < 2);
    if (ovl666_preview_phase == 2) {
        DrawUiRow(5);
        DrawUiRow(6);
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
        if (ovl666_course_counts[CourseListIndex] >= 2) {
            DrawMenuStepArrows(0xA0, 0x90, 8, 1, ovl666_selected_course.word, 0,
                          ovl666_course_counts[CourseListIndex]);
        }
    }
end:
    UpdateScreenFade(0x10036);
}
