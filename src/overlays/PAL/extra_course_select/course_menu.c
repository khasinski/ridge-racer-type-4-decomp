#include "rr/overlay666.h"

void extra_course_select_init_menu(void) {
    s32 i;
    ModeFrame = 0;
    ScreenFrame = 0;
    MenuCursor = 0;
    MenuItemCount = 3;
    ResetScreenCounters();
    MenuArenas.second = CdDirectoryBuffer + 0x2D000;
    InitMenuArenas();
    LoadCursor = MenuArenas.second;
    for (i = 0; i < 4; i++)
        InitGlyphRow(i, -2, 0, 1);
    InitRect16Row(GlyphRows->glyph07, GlyphRows->x08, GlyphRows->y0A, GlyphRows->w0C,
                  GlyphRows->h0E, 0, 0);
    InitUiRow(0, 12, 64, 68, 32, 0, 0, 1, 0, 83);
    InitUiRow(1, 12, 104, 68, 32, 0, 0, 1, 0, 85);
    InitUiRow(2, 12, 368, 248, 32, 0, 0, 4, 12, CourseListIndex + 8);
    InitUiRow(3, 24, 88, 68, 32, 0, 0, 1, 0, 84);
    InitUiRow(4, 24, 128, 68, 32, 0, 0, 1, 0, 85);
    InitUiRow(5, 116, 88, 28, 32, 0, 0, 1, 0, 55);
    InitUiRow(6, 116, 128, 28, 32, 0, 0, 1, 0, 56);
}

void extra_course_select_init_scene(void) {
    s32 i;
    s32 j;

    SetDispMask(0);
    ClearScreenOverlayState();
    SetupDrawDispEnvs(0, 0, 0, 2);
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 9; j++) {
            extra_course_select_available_courses[i][j].glyph = -1;
        }
    }
    for (i = 0; i < 4; i++) {
        extra_course_select_course_counts[i] = 0;
        for (j = 0; j < 9; j++) {
            UnpackChoiceGlyph((s8 *)&SelectionRows, CourseChoiceTable[1][j].glyph);
            if (SelectionRows.selection.variant == i && SelectionRows.selection.pad02[0] < 39) {
                extra_course_select_available_courses[i][extra_course_select_course_counts[i]] = CourseChoiceTable[1][j];
                extra_course_select_course_counts[i]++;
            }
        }
    }
    CourseMenuSelection = 0;
    CourseListIndex = 0;
    DebugPrintStub(extra_course_select_scene_name);
    extra_course_select_init_menu();
    ModeGroup = 1;
    ModeId = 79;
    ModeSubState = 0;
    SeqStartTrack(0);
}

void extra_course_select_update_menu(void) {
    s32 sound;
    s16 glyphIndex;
    RrMenuGlyphRow1C *glyphBase;
    s32 selected;
    s32 ready;
    s16 timer;
    s32 glyphCount;
    s32 choiceRow;

    s32 closing;
    s8 cellState;

    ModeFrame = (s32)((u32)ModeFrame + 1U);
    ScreenFrame = (s32)((u32)ScreenFrame + 1U);
    ready = LoadGlyphRowsStep();
    if (ready == 1) {
        switch (ModeSubState) {
        case 0:
            glyphCount = 0;
            do {
                GlyphRows[glyphCount].phase18 =
                    (s32)((u32)GlyphRows[glyphCount].phase18 + 0x30000U);
                glyphCount++;
            } while (glyphCount < 4);
            ModeSubState = 1;
            goto title;
        case 1:
            SetMenuFrameOptions(1, 1, 1);
            UiRows[MenuCursor].cells[0] = 0;
            StepMenuValue(0, 2, (char *)&MenuCursor, 2, 0);
            cellState = 2;
            if (extra_course_select_course_counts[CourseListIndex] <= 0) {
                UiRows[0].half20 = ready;
                if (MenuCursor == 0) {
                    cellState = 4;
                }
            } else {
                UiRows[0].half20 = 0;
            }
            UiRows[MenuCursor].cells[0] = cellState;
            extra_course_select_update_course_selection();
            closing = 0;
            if (PlayerControllers[0].a != 0) {
                selected = MenuCursor;
                if (selected == 0) {
                    if (extra_course_select_course_counts[CourseListIndex] > 0) {
                        closing = 1;
                        ModeSubState = 3;
                        PlaySfx(1);
                        choiceRow = 0;

                        do {
                            if (UiRows[choiceRow].byte14 == 2) {
                                UiRows[choiceRow].byte14 = 3U;
                            }
                            choiceRow += 1;
                        } while (choiceRow < 3);
                    } else {
                        PlaySfx(4);
                    }
                } else {
                    if (selected != 1)
                        goto decision;
                    sound = 1;
                    goto play_cancel;
                }
            } else {
                sound = 2;
                if (PlayerControllers[0].b != 0) {
                play_cancel:
                    PlaySfx(sound);
                    closing = 2;
                }
            }
        decision:
            if (closing > 0) {
                if (closing == 2) {
                    choiceRow = 0;

                    do {
                        if (UiRows[choiceRow].byte14 == 2) {
                            UiRows[choiceRow].byte14 = 3U;
                        }
                        choiceRow += 1;
                    } while (choiceRow < 3);
                    ModeSubState = 8;
                    MenuFadeTimer = 0x40;
                    SeqFadeVolume(0, 0x3C);
                }
            }
            goto title;
        case 3:
            SetMenuFrameOptions(0, 1, 1);
            if (UiRows[0].byte14 == 5) {
                extra_course_select_init_preview();
                ModeGroup = ready;
                ModeId = 0x50;
                ModeSubState = 0;
            } else {
                goto title;
            }
            break;
        case 8:
            SetMenuFrameOptions(0, 1, 1);
            timer = (u16)MenuFadeTimer - 1;
            MenuFadeTimer = (u16)timer;
            AddScreenFadeTiles(&CurrentDrawBuf->ordering.ordering70[0], (0x40 - timer) * 4, 0x45);
            if ((s16)MenuFadeTimer <= 0) {
                ModeGroup = ready;
                ModeId = 3;
                ModeSubState = 0;
                SeqMuteVolume();
            }
            goto update_rows;
        default:
        title:
            DrawMenuStepArrows(0xA0, 0xCC, 0x30, 0, (s32)CourseListIndex, 0, 4);
            glyphIndex = CourseListIndex;
            if ((u8)SaveProgress.prefix.flags5C[glyphIndex] != 0) {
                glyphBase = GlyphRows;

                extra_course_select_draw_selection_pulse(glyphBase[glyphIndex].x08 + 0x14,
                                            glyphBase[glyphIndex].y0A + 0x28);
                UpdateGlyphRow(CourseListIndex);
            } else {
                UpdateGlyphRow(glyphIndex);
            }
            goto update_rows;
        }
    } else {
    update_rows:
        DrawUiRow(0);
        DrawUiRow(1);
        DrawUiRow(2);
    }
    UpdateScreenFade(0x10092);
}

void extra_course_select_update_course_selection(void) {
    {
        RrUiRow24 *ui = UiRows;
        s32 selected = CourseListIndex;
        ui[2].cells[selected] = 0;
    }
    if (Pad0RepeatButtons & 0x2000) {
        if (CourseListIndex < 3) {
            CourseListIndex++;
            MenuStepDirection = 2;
            goto changed;
        }
    }
    if (Pad0RepeatButtons & 0x8000) {
        if (CourseListIndex > 0) {
            CourseListIndex--;
            MenuStepDirection = 1;
            goto changed;
        }
    }
    MenuStepDirection = 0;
    goto active;
changed:
    PlaySfx(0);
    if (CourseListIndex < 0)
        CourseListIndex = 0;
    else if (CourseListIndex >= 4)
        CourseListIndex = 3;
    GlyphRows[CourseListIndex].control02 = -6;
    GlyphRows[CourseListIndex].flags00 = 0;
    UiRows[2].byte16 = (u8)CourseListIndex + 8;
active:
    {
        RrUiRow24 *ui = UiRows;
        s32 selected = CourseListIndex;
        ui[2].cells[selected] = 2;
    }
}

void extra_course_select_draw_selection_pulse(s32 x, s32 y) {
    RrGpuScratchPointers *scratch = RR_GPU_SCRATCH;
    u8 *cursor = scratch->cursor;
    s32 wave = rsin(((u32)ScreenFrame << 5) & 0xFE0) / 32;
    u16 mode = 57;
    if (wave < 0) {
        mode = 89;
        wave = -wave;
    }
    cursor = AddSpritePrim(CurrentDrawBuf->ordering.ordering70, cursor, x, y, &extra_course_select_selection_sprite,
                           0x7FA4, ((wave + 1) * 2) | 1);
    cursor = AddDrawModePrim(CurrentDrawBuf->ordering.ordering70, cursor, mode);
    scratch->cursor = cursor;
}
