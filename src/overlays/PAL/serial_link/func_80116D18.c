#include "rr/menu_overlay_runtime.h"

/* Four-player selection: per-pair page, row and course group tables. */
extern RrCourseChoice4 CourseMenuChoices[2][2][9];
extern s16 CourseMenuPhase;
extern s16 ChoiceGlyphCounts[2][2];
extern s16 CourseMenuCursor[2];
extern s16 CourseMenuPage[2];
extern s8 LinkEntryState[];
extern s32 UiRowCount;
/* Resident word reached through its enclosing array object (aliases row stores). */
extern s32 MenuActivePlayerPlusOne[1];
extern s16 MenuResultCode;
extern s16 LinkRoleSlot;
extern u8 serial_link_page_item_counts[4];
extern s8 serial_link_page_action_rows[2][4];
extern s8 serial_link_page_ui_rows[2][2][4];
s32 ExitOnLinkLost(void);
void UpdateLinkHandshake(s32);
void serial_link_load_player_choice(void);
void serial_link_init_course_select(void);

void serial_link_update_car_select(void) {
    RrPreviewColor color;
    s16 modelReady;
    s32 drawFlags;
    s32 drawModel;
    s32 scale;
    s32 loadResult;
    s32 previous;
    s32 state;
    s32 i;
    s32 cancel;
    s32 row;
    u16 player;
    u16 page;
    u16 other;
    s8 brightness;
    u8 closing;

    modelReady = 0;
    ModeFrame++;
    if (ExitOnLinkLost() != 0) {
        SeqMuteVolume();
        return;
    }
    UpdateLinkHandshake(2);
    PlayerCars[SelectionPlayer & 1].rot54 = (PlayerCars[SelectionPlayer & 1].rot54 + 8) & 0xFFF;
    SetupMenuCarCamera(1);
    SetupViewport(6);
    loadResult = LoadCarRangeStep(SelectionPlayer, 1);
    if (loadResult != 1) {
        if ((s8)ImageRows[1] == 1) {
            ImageRows[1] = 0;
        }
        goto draw_rows;
    }
    SetCarBounds(&PlayerCars[SelectionPlayer & 1]);
    if ((s8)ImageRows[1] == 7) {
        ImageRows[1] = loadResult;
    }
    modelReady = 1;
    switch (ModeSubState) {
    case 7:
        row = serial_link_page_ui_rows[SelectionPlayer & 1][(s16)(CourseMenuPage[SelectionPlayer & 1] ^ 1)][0];
        if (UiRows[row].byte14 == 5) {
            ModeSubState = 0;
        }
    case 0:
        SetMenuFrameOptions(1, 1, 1);
        previous = PackChoiceGlyph((s8 *)&SelectionRows + SelectionPlayer * 4);
        StepMenuValue(((SelectionPlayer & 1) * 4) | 1, 2, (char *)&CourseMenuCursor[SelectionPlayer & 1],
                      ChoiceGlyphCounts[SelectionPlayer & 1][CourseMenuPage[SelectionPlayer & 1]], 0);
        UnpackChoiceGlyph((s8 *)&SelectionRows + SelectionPlayer * 4,
                      CourseMenuChoices[SelectionPlayer & 1][CourseMenuPage[SelectionPlayer & 1]][CourseMenuCursor[SelectionPlayer & 1]].glyph);
        if (((u32)previous << 16) != ((u32)PackChoiceGlyph((s8 *)&SelectionRows + SelectionPlayer * 4) << 16)) {
            ResetScreenCounters();
            LoadCursor = MenuArenas.first;
            UnpackChoiceGlyph((s8 *)&SelectionRows + SelectionPlayer * 4,
                          CourseMenuChoices[SelectionPlayer & 1][CourseMenuPage[SelectionPlayer & 1]][CourseMenuCursor[SelectionPlayer & 1]].glyph);
            PlayerCourseChoices[SelectionPlayer].first = CourseMenuChoices[SelectionPlayer & 1][CourseMenuPage[SelectionPlayer & 1]][CourseMenuCursor[SelectionPlayer & 1]].first;
            PlayerCourseChoices[SelectionPlayer].second = CourseMenuChoices[SelectionPlayer & 1][CourseMenuPage[SelectionPlayer & 1]][CourseMenuCursor[SelectionPlayer & 1]].second;
            ImageRows[1] = 6;
        }
        row = serial_link_page_ui_rows[SelectionPlayer & 1][CourseMenuPage[SelectionPlayer & 1]][MenuCursor];
        UiRows[row].cells[0] = 0;
        StepMenuValue((SelectionPlayer & 1) * 4, 2, (char *)&MenuCursor,
                      serial_link_page_item_counts[CourseMenuPage[SelectionPlayer & 1]], 0);
        row = serial_link_page_ui_rows[SelectionPlayer & 1][CourseMenuPage[SelectionPlayer & 1]][MenuCursor];
        UiRows[row].cells[0] = 2;
        state = UiRows[row].byte14;
        cancel = 0;
        if (state != 2) {
            break;
        }
        if (BadgeRows->state10 != 3) {
            break;
        }
        if (PlayerControllers[SelectionPlayer & 1].a != 0) {
            PlaySfx(1);
            UiRows[row].cells[0] = 1;
            if (MenuCursor == serial_link_page_action_rows[CourseMenuPage[SelectionPlayer & 1]][0]) {
                CourseMenuPhase = state;
                ModeSubState = 4;
            } else if (MenuCursor == serial_link_page_action_rows[CourseMenuPage[SelectionPlayer & 1]][1] && CourseMenuChoices[SelectionPlayer][1][0].glyph != -1) {
                CourseMenuCursor[SelectionPlayer & 1] = 0;
                MenuCursor = 0;
                for (i = 0; i < 4; i++) {
                    row = serial_link_page_ui_rows[SelectionPlayer & 1][CourseMenuPage[SelectionPlayer & 1]][i];
                    if (row == -1) {
                        break;
                    }
                    if (UiRows[row].byte14 == 2) {
                        UiRows[row].byte14 = 3;
                    }
                }
                for (i = 0; i < 4; i++) {
                    row = serial_link_page_ui_rows[SelectionPlayer & 1][(s16)(CourseMenuPage[SelectionPlayer & 1] ^ 1)][i];
                    if (row == -1) {
                        break;
                    }
                    UiRows[row].byte14 = 0;
                }
                ModeSubState = 7;
                CourseMenuPage[SelectionPlayer & 1] ^= 1;
            } else if (MenuCursor == serial_link_page_action_rows[CourseMenuPage[SelectionPlayer & 1]][2]) {
                cancel = 1;
            } else if (MenuCursor == serial_link_page_action_rows[CourseMenuPage[SelectionPlayer & 1]][3]) {
                for (i = 0; i < UiRowCount; i++) {
                    if (UiRows[i].byte14 == 2) {
                        UiRows[i].byte14 = 3;
                    }
                }
                ImageRows[1] = 3;
                MenuFadeTimer = 0x40;
                ModeSubState = 10;
                BadgeRows->state10 = 4;
            }
        } else if (PlayerControllers[SelectionPlayer & 1].b != 0) {
            PlaySfx(2);
            cancel = 1;
        }
        if (cancel == 1) {
            for (i = 0; i < UiRowCount; i++) {
                if (UiRows[i].byte14 == 2) {
                    UiRows[i].byte14 = 3;
                }
            }
            ImageRows[1] = 3;
            MenuFadeTimer = 0x40;
            ModeSubState = 8;
            BadgeRows->state10 = 4;
            SeqFadeVolume(0, 0x3C);
        }
        break;
    case 3:
        row = serial_link_page_ui_rows[(SelectionPlayer - 1) & 1][1][3] + 1;
        if (UiRows[row].byte14 == 5) {
            CourseMenuPhase = 1;
            ModeSubState = 0;
        }
        break;
    case 4:
        row = serial_link_page_ui_rows[SelectionPlayer & 1][1][3] + 1;
        state = UiRows[row].byte14;
        if (state != 2) {
            break;
        }
        UiRows[row + ((RrMenuChoice4 *)&SelectionRows)[SelectionPlayer].bytes[3]].cells[0] = 0;
        StepMenuValue((SelectionPlayer & 1) * 4, 1, (char *)&((RrMenuChoice4 *)&SelectionRows)[SelectionPlayer].bytes[3], 2, 0);
        UiRows[row + ((RrMenuChoice4 *)&SelectionRows)[SelectionPlayer].bytes[3]].cells[0] = state;
        if (PlayerControllers[SelectionPlayer & 1].a != 0) {
            PlaySfx(1);
            UiRows[row + ((RrMenuChoice4 *)&SelectionRows)[SelectionPlayer].bytes[3]].cells[0] = 1;
            UiRows[row].byte14 = 3;
            UiRows[row + 1].byte14 = 3;
            for (i = 0; i < 4; i++) {
                row = serial_link_page_ui_rows[SelectionPlayer & 1][CourseMenuPage[SelectionPlayer & 1]][i];
                if (row == -1) {
                    break;
                }
                if (UiRows[row].byte14 == 2) {
                    UiRows[row].byte14 = 3;
                }
            }
            modelReady = 0;
            MenuArenas.first = LoadCursor;
            if ((SelectionPlayer & 1) > 0 || LinkEntryState[(1 - LinkRoleSlot) * 2 + 1] != 1) {
                ModeSubState = 5;
                for (i = 0; i < UiRowCount; i++) {
                    if (UiRows[i].byte14 == 2) {
                        UiRows[i].byte14 = 3;
                    }
                }
                ImageRows[1] = 3;
                BadgeRows->state10 = 4;
                break;
            }
            ModeSubState = 3;
            SelectionPlayer = SelectionPlayer + 1;
            serial_link_load_player_choice();
        } else if (PlayerControllers[SelectionPlayer & 1].b != 0) {
            PlaySfx(2);
            UiRows[row].byte14 = 3;
            UiRows[row + 1].byte14 = 3;
            ModeSubState = 6;
        }
        break;
    case 5:
        SetMenuFrameOptions(0, 1, 1);
        row = serial_link_page_ui_rows[SelectionPlayer & 1][1][3] + 1;
        closing = UiRows[row].byte14;
        if (closing == 5 && (s8)ImageRows[1] == closing) {
            ResetScreenCounters();
            serial_link_init_course_select();
            ModeGroup = 1;
            ModeId = 61;
            ModeSubState = 0;
        }
        break;
    case 6:
        row = serial_link_page_ui_rows[SelectionPlayer & 1][1][3] + 1;
        if (UiRows[row].byte14 == 5) {
            ModeSubState = 0;
            CourseMenuPhase = 1;
            UiRows[row].byte14 = 0;
            UiRows[row + 1].byte14 = 0;
        }
        break;
    case 8:
        SetModelTexturePage(-1);
        SetMenuFrameOptions(0, 1, 1);
        MenuFadeTimer = (u16)MenuFadeTimer - 1;
        AddScreenFadeTiles(&CurrentDrawBuf->ordering.ordering70[0], (0x40 - MenuFadeTimer) * 4, 0x45);
        if (MenuFadeTimer <= 0) {
            SeqMuteVolume();
            ModeGroup = 1;
            ModeId = 3;
            ModeSubState = 0;
        }
        break;
    case 10:
        SetModelTexturePage(-1);
        SetMenuFrameOptions(0, 1, 1);
        MenuFadeTimer = (u16)MenuFadeTimer - 1;
        AddScreenFadeTiles(&CurrentDrawBuf->ordering.ordering70[0], (0x40 - MenuFadeTimer) * 4, 0x45);
        if (MenuFadeTimer <= 0) {
            MenuResultCode = 2;
            ModeGroup = 1;
            ModeId = 71;
            ModeSubState = 0;
        }
        break;
    }
draw_rows:
    if (CourseMenuPhase == 0 && (s8)ImageRows[1] == 2) {
        CourseMenuPhase = 1;
        BadgeRows->half02 = 1;
    }
    if (ModeSubState == 3) {
        player = (u16)SelectionPlayer - 1;
    } else {
        player = SelectionPlayer;
    }
    if (ModeSubState == 7) {
        page = CourseMenuPage[player & 1];
        other = page ^ 1;
        row = serial_link_page_ui_rows[player & 1][(s16)other][0];
        if (UiRows[row].byte14 == 5) {
            other = page;
        }
    } else {
        other = CourseMenuPage[player & 1];
    }
    for (i = 0; i < 4; i++) {
        row = serial_link_page_ui_rows[player & 1][(s16)other][i];
        if (row == -1) {
            break;
        }
        DrawUiRow(row);
    }
    if (CourseMenuPhase == 2) {
        row = serial_link_page_ui_rows[player & 1][1][3];
        DrawUiRow(row + 1);
        DrawUiRow(row + 2);
    }
    if ((u32)((u8)ImageRows[1] - 1) < 2U) {
        drawModel = modelReady;
        if (drawModel == 1) {
            DrawSelectionIconGrid(SelectionPlayer);
            PrepareCarDrawPose(&PlayerCars[SelectionPlayer & 1]);
            scale = 0x100;
            if ((s8)ImageRows[1] == drawModel) {
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
            DrawPreviewCar(&PlayerCars[SelectionPlayer & 1], color.argument, scale, drawFlags);
        }
    }
    UpdateBadgeRowAnim(0);
    UpdateImageRow(0);
    MenuActivePlayerPlusOne[0] = SelectionPlayer + 1;
    UpdateScreenFade(CourseMenuPage[SelectionPlayer & 1] != 0 ? 0x10036 : 0x10032);
}
