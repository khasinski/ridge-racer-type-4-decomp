#include "rr/overlay662_course_controller.h"
void ovl662_func_801154C8(void) {
    union {RrPreviewRgb3 rgb; u8 channel[3];} color;
    s32 var_a3;
    s32 temp_s1;
    s16 temp_v1_7;
    s16 temp_v1_8;
    s32 temp_a1;
    s16 closePhase;
    s32 temp_s0;
    s32 temp_s5;
    s32 var_a2_2;
    s32 var_s0;
    s8 var_s6;
    s32 temp_v0_2;
    s8 temp_v0_3;
    s32 var_a1;
    u8 temp_s3;
    RrRaceVehicle *temp_v0;
    RrRaceVehicle *vehicleBase;
    s32 playerIndex;
    s32 *tick;
    u8 *temp_v1;
    RrUiRow24 *temp_v1_2;
    RrUiRow24 *temp_v1_3;
    RrUiRow24 *temp_v1_4;
    RrUiRow24 *temp_v1_5;
    RrUiRow24 *temp_v1_6;

    var_s6 = 0;
    tick = &ModeFrame;
    playerIndex = SelectionPlayer;
    vehicleBase = PlayerCars;
    temp_v0 = &vehicleBase[playerIndex&1];
    temp_a1 = ((u32)temp_v0->rot54 + 8) & 0xFFF;
    *tick=(u32)*tick+1;
    temp_v0->rot54 = temp_a1;
    SetupMenuCarCamera(1);
    SetupViewport(6);
    temp_v0_2 = LoadCarRangeStep(0, 1);
    if (temp_v0_2 != 1) {
        if ((*(s8*)(ImageRows+1)) == 1) {
            (*(s8*)(ImageRows+1)) = 0;
        }
        goto block_67;
    }
    SetCarBounds(vehicleBase);
    if ((*(s8*)(ImageRows+1)) == 7) {
        (*(s8*)(ImageRows+1)) = temp_v0_2;
    }
    var_s6 = 1;
    switch ((u16)D_800F4D8A) {
    case 0:
        SetMenuFrameOptions(1, 1, 1);
        temp_s0 = PackChoiceGlyph((s8*)&SelectionRows);
        StepMenuValue(1, 2, (char*)&ovl662_D_801272E4, ovl662_D_801272E0, 0);
        UnpackChoiceGlyph((s8*)&SelectionRows, CourseChoiceTable[1][ovl662_D_801272E4].glyph);
        if (((u32)temp_s0 << 0x10) != ((u32)PackChoiceGlyph((s8*)&SelectionRows) << 0x10)) {
            ResetScreenCounters();
            LoadCursor = MenuArenas.first;
            UnpackChoiceGlyph((s8*)&SelectionRows, CourseChoiceTable[1][ovl662_D_801272E4].glyph);
            (*(s8*)(ImageRows+1)) = 6;
            temp_v1 = (u8 *)(void *)&CourseChoiceTable + ovl662_D_801272E4 * sizeof(RrCourseChoice4);
            PlayerCourseChoices[0].first = (s16) (s8) ((RrCourseChoice4 *)(void *)(temp_v1 + sizeof(CourseChoiceTable[0])))->first;
            PlayerCourseChoices[0].second = (s16) (s8) ((RrCourseChoice4 *)(void *)(temp_v1 + sizeof(CourseChoiceTable[0])))->second;
        }
        UiRows[MenuCursor].cells[0] = 0;
        StepMenuValue(0, 2, (char*)&MenuCursor, 4, 0);
        var_a1 = 2;
        temp_s5 = ((SelectionRows.pad02[0] % 20) & 0xFF) == 0x13;
        if (temp_s5 != 0) {
            if (ovl662_D_801272E4 == 8) {
                UiRows[0].half20 = 1;
                if (MenuCursor == 0) {
                    var_a1 = 4;
                }
            } else {
                UiRows[0].half20 = 0;
            }
            UiRows[2].half20 = 1;
            if (MenuCursor == 2) {
                var_a1 = 4;
            }
        } else {
            UiRows[0].half20 = 0;
            UiRows[2].half20 = 0;
        }
        UiRows[MenuCursor].cells[0] = var_a1;
        temp_s3 = UiRows[0].byte14;
        if (temp_s3 == 2) {
            temp_s1 = BadgeRows->state10;
            if (temp_s1 == 3) {
                if (PlayerControllers[0].a != 0) {
                    SetMenuFrameOptions(0, 1, 1);
                    UiRows[MenuCursor].cells[0] = 1;
                    if (MenuCursor == 0) {
                        if (ovl662_D_801272E4 < 8) {
                            SetModelTexturePage(-1);
                            PlaySfx(1);
                            var_s0 = 0;

                            D_800F4D8A = (u16) temp_s1;
                            BadgeRows->state10 = 4;
                            closePhase = 3;
                            __asm__("" : "+r"(closePhase));
                            (*(s8*)(ImageRows+1)) = closePhase;
                            do {
                                temp_v1_2 = (RrUiRow24 *)((unsigned long)var_s0 * sizeof(RrUiRow24) + (unsigned long)UiRows);
                                if (temp_v1_2->byte14 == 2) {
                                    temp_v1_2->byte14 = 3U;
                                }
                                var_s0 += 1;

                            } while (var_s0 < 4);
                        } else {
                            goto block_40;
                        }
                    } else if (MenuCursor == 1) {
                        SetModelTexturePage(-1);
                        PlaySfx(1);
                        var_s0 = 0;

                        BadgeRows->state10 = 4;
                        (*(s8*)(ImageRows+1)) = (s8) temp_s1;
                        do {
                            temp_v1_3 = (RrUiRow24 *)((unsigned long)var_s0 * sizeof(RrUiRow24) + (unsigned long)UiRows);
                            if (temp_v1_3->byte14 == 2) {
                                temp_v1_3->byte14 = 3U;
                            }
                            var_s0 += 1;

                        } while (var_s0 < 4);
                        MenuFadeTimer = 0x40;
                        D_800F4D8A = 5;
                    } else if (MenuCursor == temp_s3) {
                        if (temp_s5 == 0) {
                            SetModelTexturePage(-1);
                            PlaySfx(1);
                            var_s0 = 0;

                            D_800F4D8A = 4;
                            BadgeRows->state10 = 4;
                            (*(s8*)(ImageRows+1)) = (s8) temp_s1;
                            do {
                                temp_v1_4 = (RrUiRow24 *)((unsigned long)var_s0 * sizeof(RrUiRow24) + (unsigned long)UiRows);
                                if (temp_v1_4->byte14 == 2) {
                                    temp_v1_4->byte14 = 3U;
                                }
                                var_s0 += 1;

                            } while (var_s0 < 4);
                        } else {
block_40:
                            PlaySfx(4);
                        }
                    } else if (MenuCursor == temp_s1) {
                        SetModelTexturePage(-1);
                        PlaySfx(1);
                        var_s0 = 0;
                        if (UiRowCount > 0) {

                            do {
                                temp_v1_5 = (RrUiRow24 *)((unsigned long)var_s0 * sizeof(RrUiRow24) + (unsigned long)UiRows);
                                if (temp_v1_5->byte14 == 2) {
                                    temp_v1_5->byte14 = 3U;
                                }
                                var_s0 += 1;

                            } while (var_s0 < UiRowCount);
                        }
                        BadgeRows->state10 = 4;
                        (*(s8*)(ImageRows+1)) = 3;
                        goto block_54;
                    }
                } else if (PlayerControllers[0].b != 0) {
                    PlaySfx(2);
                    SetModelTexturePage(-1);
                    SetMenuFrameOptions(0, 1, 1);
                    var_s0 = 0;
                    if (UiRowCount > 0) {

                        do {
                            temp_v1_6 = (RrUiRow24 *)((unsigned long)var_s0 * sizeof(RrUiRow24) + (unsigned long)UiRows);
                            if (temp_v1_6->byte14 == 2) {
                                temp_v1_6->byte14 = 3U;
                            }
                            BadgeRows->state10 = 4;
                            (*(s8*)(ImageRows+1)) = 3;
                            var_s0 += 1;

                        } while (var_s0 < UiRowCount);
                    }
block_54:
                    MenuFadeTimer = 0x40;
                    D_800F4D8A = 8;
                }
            }
        }
        goto block_67;
    case 3:
        if (((*(s8*)(ImageRows+1)) == 5) && (BadgeRows->state10 == 0xB)) {
            ModeGroup = 1;
            ModeId = 0x41;
            D_800F4D8A = 0;
            ovl662_D_801272BA = (u16) ovl662_D_801272E4;
        }
        goto block_67;
    case 4:
        if (((*(s8*)(ImageRows+1)) == 5) && (BadgeRows->state10 == 0xB)) {
            ovl662_func_80118E6C();
        } else {
            goto block_67;
        }
        goto done;
    case 5:
        temp_v1_7 = MenuFadeTimer - 1;
        MenuFadeTimer = (u16) temp_v1_7;
        AddScreenFadeTiles(((RrFrameStorage*)(void*)CurrentDrawBuf)->ordering.ordering70, (0x40 - temp_v1_7) * 4, 0x45);
        if ((s16) MenuFadeTimer <= 0) {
            ModeGroup = 1;
            ModeId = 0x1B;
            D_800F4D8A = 0;
        }
        goto block_67;
    case 8:
        temp_v1_8 = MenuFadeTimer - 1;
        MenuFadeTimer = (u16) temp_v1_8;
        AddScreenFadeTiles(((RrFrameStorage*)(void*)CurrentDrawBuf)->ordering.ordering70, (0x40 - temp_v1_8) * 4, 0x45);
        if ((s16) MenuFadeTimer <= 0) {
            ModeGroup = 1;
            ModeId = 0x27;
            D_800F4D8A = 0;
        }
        goto block_67;
    default:
        break;
    }
block_67:
        if ((ovl662_D_801272D4 == 0) && ((*(s8*)(ImageRows+1)) == 2)) {
            ovl662_D_801272D4 = 1;
            BadgeRows->half02 = 1;
            var_s0 = 0;
        }
        var_s0 = 0;
        do {
            DrawUiRow(var_s0);
            var_s0 += 1;
        } while (var_s0 < 4);
        var_s0 = var_s6;
        if (((u32) ((u8) (*(s8*)(ImageRows+1)) - 1) < 2U) && (var_s0 == 1)) {
            DrawSelectionIconGrid(0);
            func_8002E5A0((char *)(void *)PlayerCars);
            var_a2_2 = 0x100;
            if ((*(s8*)(ImageRows+1)) == var_s0) {
                temp_v0_3 = ~((ImageRows[2] * 0xFF) / 30);
                color.channel[0] = temp_v0_3;
                color.channel[1] = temp_v0_3;
                color.channel[2] = temp_v0_3;
                var_a3 = 0x102;
                var_a2_2 = (ImageRows[2] << 8) / 30;
            } else {
                var_a3 = 1;
                color.channel[0] = 0x80;
                color.channel[1] = 0x80;
                color.channel[2] = 0x80;
            }
            DrawPreviewCar(PlayerCars, color.rgb, var_a2_2, var_a3);
        }
        UpdateBadgeRowAnim(0);
        if (UpdateImageRow(0) > 0) {
            DrawMenuStepArrows(0xA0, 0x90, 8, 1, (s32) ovl662_D_801272E4, 0, (s32) ovl662_D_801272E0);
        }
done:
    UpdateScreenFade(0x1008E);
}
