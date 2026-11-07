#include "rr/records.h"

void records_init_menu(void) {
    ModeFrame = 0;
    ScreenFrame = 0;
    MenuCursor = 0;
    MenuItemCount = 3;
    ClearScreenOverlayState();
    records_extra_courses_enabled = 0;
    records_record_kind = 0;
    ResetScreenCounters();
    MenuArenas.second = CdDirectoryBuffer + 0x2D000;
    InitMenuArenas();
    LoadCursor = MenuArenas.second;
    InitUiRow(0, 118, 152, 68, 32, 0, 0, 1, 0, 122);
    InitUiRow(1, 118, 192, 68, 32, 0, 0, 1, 1, 16);
    InitUiRow(2, 118, 232, 68, 32, 0, 0, 1, 1, 17);
    InitUiRow(3, 118, 272, 68, 32, 0, 0, 1, 0, 85);
    if (!records_available_course_count)
        UiRows->half20 = 1;
    SetupRenderer(5);
}

void records_init_scene(void) {
    s32 i;
    s32 val;

    SetDispMask(0);
    SetupDrawDispEnvs(0, 0, 0, 2);
    ModeGroup = 1;
    ModeId = 74;
    ModeSubState = 0;
    DebugPrintStub(records_scene_name);
    records_available_course_count = 0;
    for (i = 0; i < 9; i++) {
        if (SaveProgress.unlocked60[records_course_order[i]] != 0) {
            records_available_courses[records_available_course_count] = records_course_order[i];
            records_course_unlock_levels[records_available_course_count++] =
                SaveProgress.unlocked60[records_course_order[i]];
        }
    }
    records_init_menu();
    SeqStartTrack(0);
    records_course_count = 4;
    val = SaveProgress.prefix.progress.prefix00.fields.configuration0C;
    if (val >= 16) {
        records_course_count = 8;
    }
    if (val & 0x100) {
        records_course_count = 16;
    }
}

void records_update_menu(void) {
    s32 cancelSound;
    s16 selected;
    s32 loaded;
    s16 closingTimer;
    s16 cancelTimer;
    s32 i;

    s32 closingMode;
    s32 highlight;
    u8 rowState;

    ModeFrame = (s32)((u32)ModeFrame + 1U);
    ScreenFrame = (s32)((u32)ScreenFrame + 1U);
    switch (ModeSubState) {
    case 0:
        i = 0;
        if (UiRows->byte14 == 2) {
            SetMenuFrameOptions(1, 1, 1);
            UiRows[MenuCursor].cells[0] = 0;
            StepMenuValue(0, 2, (char *)&MenuCursor, 4, 0);
            highlight = 2;
            if (records_available_course_count == 0) {
                if (MenuCursor == 0) {
                    highlight = 4;
                }
            }
            UiRows[MenuCursor].cells[0] = highlight;
            closingMode = 0;
            if (PlayerControllers[0].a != 0) {
                UiRows[MenuCursor].cells[0] = 1;
                selected = MenuCursor;
                if (selected == 0) {
                    if (records_available_course_count > 0) {
                        PlaySfx(1);
                        closingMode = 2;
                        ResetScreenCounters();
                        ModeSubState = 3;
                        LoadCursor = MenuArenaBase;
                    } else {
                        PlaySfx(4);
                    }
                    goto chosen;
                } else if (selected == 1) {
                    PlaySfx(1);
                    closingMode = 2;
                    records_record_kind = selected;
                    ModeSubState = 4;
                    goto chosen;
                } else if (selected == 2) {
                    PlaySfx(1);
                    records_record_kind = selected;
                    closingMode = 2;
                    ModeSubState = 5;
                    goto chosen;
                } else {
                    if (selected != 3)
                        goto chosen;
                    cancelSound = 1;
                    goto cancel;
                }
            } else {
                cancelSound = 2;
                if (PlayerControllers[0].b != 0) {
                cancel:
                    PlaySfx(cancelSound);
                    ModeSubState = 8;
                    closingMode = 1;
                }
            }
        chosen:
            if (closingMode > 0) {
                SetMenuFrameOptions(0, 1, 1);
                MenuFadeTimer = 0x40;
                i = 0;

                do {
                    rowState = UiRows[i].byte14;
                    if (rowState == 2) {
                        if (closingMode == rowState) {
                            UiRows[i].byte14 = 7U;
                        } else {
                            UiRows[i].byte14 = 3U;
                            SeqFadeVolume(0, (s16)(MenuFadeTimer - 4));
                        }
                    }
                    i += 1;
                } while (i < 4);
                i = 0;
            }
        }
        break;
    case 3:
        if ((s16)MenuFadeTimer > 0) {
            MenuFadeTimer = (u16)MenuFadeTimer - 1;
        }
        AddScreenFadeTiles(CurrentDrawBuf->ordering.ordering70, (0x40 - (s16)MenuFadeTimer) * 4, 0x45);
        i = 0;
        if ((s16)MenuFadeTimer <= 0) {
            loaded = LoadCoursePreviewStep(records_available_courses[0]);
            if (loaded == 1) {
                records_init_preview();
                ModeGroup = loaded;
                ModeId = 0x4C;
                ModeSubState = 0;
                return;
            }
            break;
        }
        break;
    case 4:
    case 5:
        closingTimer = (u16)MenuFadeTimer - 1;
        MenuFadeTimer = (u16)closingTimer;
        AddScreenFadeTiles(CurrentDrawBuf->ordering.ordering70, (0x40 - closingTimer) * 4, 0x45);
        i = 0;
        if ((s16)MenuFadeTimer <= 0) {
            records_init_selection(records_record_kind);
            ModeGroup = 1;
            ModeId = 75;
            ModeSubState = 0;
        }
        break;
    case 8:
        cancelTimer = (u16)MenuFadeTimer - 1;
        MenuFadeTimer = (u16)cancelTimer;
        AddScreenFadeTiles(CurrentDrawBuf->ordering.ordering70, (0x40 - cancelTimer) * 4, 0x45);
        i = 0;
        if ((s16)MenuFadeTimer <= 0) {
            SeqMuteVolume();
            ModeGroup = 1;
            ModeId = 3;
            ModeSubState = 0;
        }
        break;
    default:
        break;
    }
    i = 0;
    do {
        DrawUiRow(i);
        i += 1;
    } while (i < 4);
    UpdateScreenFade(0x10057);
}

void records_init_selection(s32 mode) {
    s32 one = 1;
    ModeFrame = 0;
    ScreenFrame = 0;
    CourseMenuSelection = 0;
    CourseIndex = 0;
    MenuCursor = 0;
    records_selected_record = 0;
    ClearScreenOverlayState();
    InitUiRow(0, 12, 64, 68, 32, 0, 0, one, one, 24);
    InitUiRow(1, 12, 104, 68, 32, 0, 0, one, 0, 85);
    InitUiRow(2, 12, 368, 208, 32, 0, 0, 8, 4, 0);
    InitUiRow(3, 24, 88, 68, 32, 0, 0, one, 0, 83);
    if (mode == 2)
        InitBadgeRow(one, 49, 188, 222, 152, 0, 0xB04);
    else
        InitBadgeRow(one, 49, 188, 222, 152, 0, 0xA04);
    BadgeRows[1].half02 = one;
}
