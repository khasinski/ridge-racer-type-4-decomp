#include "rr/menu_overlay_runtime.h"

typedef struct {s16 value;u16 reserved;} RrGarageCarSlot;
extern s16 garage_replace_slot_index;
extern s16 garage_D_801272D4,garage_car_list_count,garage_car_index;
extern s16 garage_car_status[20];
extern RrGarageCarSlot garage_collected_cars[20];
void garage_init_car_info(void);
void garage_draw_car_status_bars(s32);

void garage_update_car_list(void)
{
    union {RrPreviewRgb3 rgb; u8 channel[3];} color;
    s8 shown;
    s32 ready;
    s32 before;
    s32 status;
    s32 state;
    s32 cell;
    s32 i;
    s32 fadeMode;
    s16 steps;
    s32 fadeLevel;
    s8 shade;
    RrRaceVehicle *vehicle;

    shown = 0;
    ModeFrame++;
    vehicle = PlayerCars;
    ScreenFrame++;
    vehicle->rot54 = (vehicle->rot54 + 8) & 0xFFF;
    SetupMenuCarCamera(1);
    SetupViewport(6);
    ready = LoadCarRangeStep(0, 1);
    if (ready != 1) {
        if ((s8)ImageRows[1] == 1) {
            ImageRows[1] = 0;
        }
        goto common;
    }
    SetCarBounds(vehicle);
    if ((s8)ImageRows[1] == 7) {
        ImageRows[1] = ready;
    }
    shown = 1;
    switch (ModeSubState) {
    case 0:
        SetMenuFrameOptions(1, 1, 1);
        before = PackChoiceGlyph((s8 *)&SelectionRows);
        StepMenuValue(3, 2, (char *)&garage_car_index, garage_car_list_count, 0);
        UnpackChoiceGlyph((s8 *)&SelectionRows, garage_collected_cars[garage_car_index].value);
        status = garage_car_status[(SelectionRows.entry.parameter % 20) & 0xFF];
        if ((s16)before != (s16)PackChoiceGlyph((s8 *)&SelectionRows)) {
            ResetScreenCounters();
            LoadCursor = MenuArenas.first;
            UnpackChoiceGlyph((s8 *)&SelectionRows, garage_collected_cars[garage_car_index].value);
            ImageRows[1] = 6;
            PlayerCourseChoices[0].first = 0;
            PlayerCourseChoices[0].second = 0;
        }
        UiRows[MenuCursor].cells[0] = 0;
        StepMenuValue(0, 2, (char *)&MenuCursor, 2, 0);
        cell = 2;
        if (status != 1) {
            UiRows[0].half20 = 1;
            if (MenuCursor == 0) {
                cell = 4;
            }
        } else {
            UiRows[0].half20 = 0;
        }
        UiRows[MenuCursor].cells[0] = cell;
        if (UiRows[0].byte14 != 2) {
            break;
        }
        state = BadgeRows[0].state10;
        if (state != 3) {
            break;
        }
        if (PlayerControllers[0].a) {
            SetMenuFrameOptions(0, 1, 1);
            UiRows[MenuCursor].cells[0] = 1;
            if (MenuCursor == 0) {
                if (status == 1) {
                    PlaySfx(1);
                    SetModelTexturePage(-1);
                    ModeSubState = 3;
                    BadgeRows[0].state10 = 4;
                    BadgeRows[1].state10 = 4;
                    ImageRows[1] = 3;
                    for (i = 0; i < 4; i++) {
                        if (UiRows[i].byte14 == 2) {
                            UiRows[i].byte14 = 3;
                        }
                    }
                    MenuFadeTimer = 64;
                } else {
                    PlaySfx(4);
                }
            } else if (MenuCursor == 1) {
                SetModelTexturePage(-1);
                PlaySfx(1);
                BadgeRows[0].state10 = 4;
                BadgeRows[1].state10 = 4;
                ImageRows[1] = state;
                for (i = 0; i < UiRowCount; i++) {
                    if (UiRows[i].byte14 == 2) {
                        UiRows[i].byte14 = 3;
                    }
                }
                MenuFadeTimer = 64;
                ModeSubState = 8;
            }
        } else if (PlayerControllers[0].b) {
            PlaySfx(2);
            SetModelTexturePage(-1);
            SetMenuFrameOptions(0, 1, 1);
            BadgeRows[0].state10 = 4;
            BadgeRows[1].state10 = 4;
            ImageRows[1] = state;
            for (i = 0; i < UiRowCount; i++) {
                if (UiRows[i].byte14 == 2) {
                    UiRows[i].byte14 = 3;
                }
            }
            MenuFadeTimer = 64;
            ModeSubState = 8;
        }
        break;
    case 3:
        MenuFadeTimer--;
        steps = 64;
        AddScreenFadeTiles(CurrentDrawBuf->ordering.ordering70, (steps - MenuFadeTimer) * 4, 69);
        if (MenuFadeTimer <= 0) {
            CourseChoiceTable[1][garage_replace_slot_index].glyph = garage_collected_cars[garage_car_index].value;
            CourseChoiceTable[1][garage_replace_slot_index].first = 0;
            CourseChoiceTable[1][garage_replace_slot_index].second = 0;
            garage_car_index = garage_replace_slot_index;
            garage_init_car_info();
            ModeGroup = ready;
            ModeId = steps;
            ModeSubState = 0;
            return;
        }
        break;
    case 8:
        MenuFadeTimer--;
        steps = 64;
        AddScreenFadeTiles(CurrentDrawBuf->ordering.ordering70, (steps - MenuFadeTimer) * 4, 69);
        if (MenuFadeTimer <= 0) {
            garage_car_index = garage_replace_slot_index;
            garage_init_car_info();
            ModeGroup = ready;
            ModeId = steps;
            ModeSubState = 0;
            return;
        }
        break;
    }
common:
    if (garage_D_801272D4 == 0 && (s8)ImageRows[1] == 2) {
        garage_D_801272D4 = 1;
        BadgeRows[0].half02 = 1;
        BadgeRows[1].half02 = 1;
    }
    for (i = 0; i < 2; i++) {
        DrawUiRow(i);
    }
    if (BadgeRows[1].state10 == 3) {
        garage_draw_car_status_bars(SelectionRows.entry.parameter);
    }
    i = shown;
    if ((u8)(ImageRows[1] - 1) < 2 && i == 1) {
        DrawSelectionIconGrid(0);
        PrepareCarDrawPose(PlayerCars);
        fadeLevel = 0x100;
        if ((s8)ImageRows[1] == i) {
            shade = ~((ImageRows[2] * 255) / 30);
            color.channel[0] = shade;
            color.channel[1] = shade;
            color.channel[2] = shade;
            fadeMode = 0x102;
            fadeLevel = (ImageRows[2] << 8) / 30;
        } else {
            fadeMode = 1;
            color.channel[0] = 0x80;
            color.channel[1] = 0x80;
            color.channel[2] = 0x80;
        }
        DrawPreviewCar(PlayerCars, color.rgb, fadeLevel, fadeMode);
    }
    UpdateBadgeRowAnim(0);
    UpdateBadgeRowAnim(1);
    if (UpdateImageRow(0) > 0) {
        DrawMenuStepArrows(160, 144, 8, 1, garage_car_index, -1, garage_car_list_count + 1);
    }
    UpdateScreenFade(0x1008E);
}
