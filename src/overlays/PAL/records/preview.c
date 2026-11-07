#include "rr/records.h"

void records_init_preview(void) {
    RrProjectionScratchPrefix *scratch;
    RrRaceVehicle *state;

    records_selected_record = 0;
    ClearScreenOverlayState();
    ScreenFadeState = 2;
    ModeFrame = 0;
    SetupRenderer(3);
    scratch = RR_PROJECTION_SCRATCH;
    scratch->word18 = 0;
    scratch->word1C = 0;
    scratch->word20 = 0;
    scratch->word08 = 0;
    scratch->word0C = 0;
    scratch->word10 = 0;
    state = PlayerCars;
    state->pos20[1] = -200;
    state->pos20[2] = 2024;
    state->pos20[0] = 0;
    state->rot50 = 40;
    state->rot54 = 0;
    state->rot58 = 0;
    InitUiRow(0, 12, 64, 68, 32, 0, 0, 1, 0, 83);
    InitBadgeRow(0, 12, 128, 76, 34, 0, 0xC01);
    BadgeRows[0].half02 = 1;
}

void records_draw_preview(void) {
    RrPreviewMatrixWorkspace mat;
    s16 rot[4];
    RrGpuRectangleFields light;
    RrRaceVehicle *state;

    BuildCameraMatrices();
    SetupViewport(6);
    state = PlayerCars;
    state->rot54 = (s32)(((u32)state->rot54 + 8U) & 0xFFFU);
    rot[0] = state->rot50;
    rot[1] = state->rot54;
    rot[2] = state->rot58;
    RotMatrix(rot, mat.halves);
    light.x = 100;
    light.y = 80;
    light.w = 120;
    light.h = 280;
    EmitTiledRect(&light, 128, CurrentDrawBuf->ordering.ordering70);
    SetSineLightColors(4096);
    SetSkyMatrixAndDraw((char *)state->pos20, mat.halves);
}

void records_update_preview(void) {
    s32 old;
    if (LoadCoursePreviewStep(records_available_courses[records_selected_record])) {
        switch (ModeSubState) {
        case 0:
            if (UiRows->byte14 == 2) {
                SetMenuFrameOptions(1, 1, 0);
                ModeSubState = 1;
            }
            break;
        case 1:
            old = records_selected_record;
            StepMenuValue(1, 4, (char *)&records_selected_record, records_available_course_count, 0);
            if (old != records_selected_record)
                ResetScreenCounters();
            else
                records_draw_preview();
            UiRows->cells[0] = 2;
            if (MenuConfirmPressed) {
                PlaySfx(1);
                SetMenuFrameOptions(0, 1, 0);
                ModeSubState = 8;
                UiRows->cells[0] = 1;
                MenuFadeTimer = 64;
                UiRows->byte14 = 3;
            }
            break;
        case 8:
            MenuFadeTimer--;
            AddScreenFadeTiles(CurrentDrawBuf->ordering.ordering70, (64 - MenuFadeTimer) * 4, 69);
            if (MenuFadeTimer <= 0) {
                records_init_menu();
                ModeGroup = 1;
                ModeId = 74;
                ModeSubState = 0;
                return;
            }
            break;
        }
    }
    if (records_available_course_count >= 2)
        DrawMenuStepArrows(160, 384, 16, 1, records_selected_record, 0, records_available_course_count);
    DrawUiRow(0);
    UpdateScreenFade(0x10058);
}

void records_init_result_preview(void) {
    RrRaceVehicle *state;
    RrProjectionScratchPrefix *scratch;
    ModeFrame = 0;
    SetupRenderer(5);
    state = PlayerCars;
    scratch = RR_PROJECTION_SCRATCH;
    scratch->word18 = 120;
    scratch->word08 = -16;
    scratch->word0C = -248;
    scratch->word1C = 0;
    scratch->word20 = 0;
    scratch->word10 = -96;
    ResetCarState(state);
    state->pos20[2] = 308;
    state->pos20[1] = 10;
    state->rot54 = 2560;
    InitPreviewImageRows();
    BadgeRows->half02 = 1;
}

void records_update_result_preview(void) {

    RrPreviewColor color;
    s32 scale;
    ModeFrame = (s32)((u32)ModeFrame + 1U);
    switch ((u16)ModeSubState) {
    case 0:
        if (UiRows[3].byte14 == 2) {
            SetMenuFrameOptions(1, 1, 0);
            ModeSubState = 1;
        }
        break;
    case 1:
        UiRows[3].cells[0] = 2;
        if (MenuConfirmPressed) {
            PlaySfx(1);
            SetMenuFrameOptions(0, 1, 0);
            UiRows[3].cells[0] = 1;
            UiRows[3].byte14 = 3;
            ModeSubState = 2;
            BadgeRows->state10 = 4;
            ImageRows[1] = 3;
        }
        break;
    case 2:
        if (BadgeRows->state10 == 11 && ((s8 *)ImageRows)[1] == 5) {
            records_init_selection(records_record_kind);
            ModeGroup = 1;
            ModeId = 75;
            ModeSubState = 0;
        }
        break;
    }
    PlayerCars[0].rot54 = (s32)(((u32)PlayerCars[0].rot54 + 8U) & 4095U);
    SetupMenuCarCamera(1);
    SetupViewport(6);
    if ((u32)ImageRows[1] - 1U < 2U) {
        RrSelectionRows *selected = &SelectionRows.selection;
        selected->pad02[1] = 2;
        DrawSelectionIconGrid(0);
        selected->pad02[1] = 0;
        PrepareCarDrawPose(PlayerCars);
        if (((s8 *)ImageRows)[1] == 1) {
            s32 level = ~(ImageRows[2] * 255 / 30);
            color.channels.r = level;
            color.channels.g = level;
            color.channels.b = level;
            scale = ImageRows[2] * 256 / 30;
        } else {
            color.channels.r = 128;
            color.channels.g = 128;
            color.channels.b = 128;
            scale = 256;
        }
        DrawPreviewCar(PlayerCars, color.argument, scale, 0);
    }
    DrawUiRow(3);
    UpdateBadgeRowAnim(0);
    UpdateImageRow(0);
    UpdateScreenFade(0x10022);
}
