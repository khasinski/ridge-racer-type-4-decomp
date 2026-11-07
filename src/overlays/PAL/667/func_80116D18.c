#include "rr/menu_overlay_runtime.h"

/* Four-player selection: per-pair page, row and course group tables. */
extern RrCourseChoice4 D_8010BFD8[2][2][9];
extern s16 D_8010C0B8;
extern s16 D_8010C0C0[2][2];
extern s16 D_8010C0D0[2];
extern s16 D_8010C0D8[2];
extern s8 LinkEntryState[];
extern s32 UiRowCount;
/* Resident word reached through its enclosing array object (aliases row stores). */
extern s32 D_800F4CB8[1];
extern s16 D_800FFC18;
extern s16 D_800AD508;
extern u8 ovl667_D_80118E10[4];
extern s8 ovl667_D_80118E14[2][4];
extern s8 ovl667_D_80118E1C[2][2][4];
s32 ExitOnLinkLost(void);
void UpdateLinkHandshake(s32);
void ovl667_func_80116BB4(void);
void ovl667_func_80117EF8(void);

void ovl667_func_80116D18(void) {
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
    switch (D_800F4D8A) {
    case 7:
        row = ovl667_D_80118E1C[SelectionPlayer & 1][(s16)(D_8010C0D8[SelectionPlayer & 1] ^ 1)][0];
        if (UiRows[row].byte14 == 5) {
            D_800F4D8A = 0;
        }
    case 0:
        SetMenuFrameOptions(1, 1, 1);
        previous = PackChoiceGlyph((s8 *)&SelectionRows + SelectionPlayer * 4);
        StepMenuValue(((SelectionPlayer & 1) * 4) | 1, 2, (char *)&D_8010C0D0[SelectionPlayer & 1],
                      D_8010C0C0[SelectionPlayer & 1][D_8010C0D8[SelectionPlayer & 1]], 0);
        UnpackChoiceGlyph((s8 *)&SelectionRows + SelectionPlayer * 4,
                      D_8010BFD8[SelectionPlayer & 1][D_8010C0D8[SelectionPlayer & 1]][D_8010C0D0[SelectionPlayer & 1]].glyph);
        if (((u32)previous << 16) != ((u32)PackChoiceGlyph((s8 *)&SelectionRows + SelectionPlayer * 4) << 16)) {
            ResetScreenCounters();
            LoadCursor = MenuArenas.first;
            UnpackChoiceGlyph((s8 *)&SelectionRows + SelectionPlayer * 4,
                          D_8010BFD8[SelectionPlayer & 1][D_8010C0D8[SelectionPlayer & 1]][D_8010C0D0[SelectionPlayer & 1]].glyph);
            PlayerCourseChoices[SelectionPlayer].first = D_8010BFD8[SelectionPlayer & 1][D_8010C0D8[SelectionPlayer & 1]][D_8010C0D0[SelectionPlayer & 1]].first;
            PlayerCourseChoices[SelectionPlayer].second = D_8010BFD8[SelectionPlayer & 1][D_8010C0D8[SelectionPlayer & 1]][D_8010C0D0[SelectionPlayer & 1]].second;
            ImageRows[1] = 6;
        }
        row = ovl667_D_80118E1C[SelectionPlayer & 1][D_8010C0D8[SelectionPlayer & 1]][MenuCursor];
        UiRows[row].cells[0] = 0;
        StepMenuValue((SelectionPlayer & 1) * 4, 2, (char *)&MenuCursor,
                      ovl667_D_80118E10[D_8010C0D8[SelectionPlayer & 1]], 0);
        row = ovl667_D_80118E1C[SelectionPlayer & 1][D_8010C0D8[SelectionPlayer & 1]][MenuCursor];
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
            if (MenuCursor == ovl667_D_80118E14[D_8010C0D8[SelectionPlayer & 1]][0]) {
                D_8010C0B8 = state;
                D_800F4D8A = 4;
            } else if (MenuCursor == ovl667_D_80118E14[D_8010C0D8[SelectionPlayer & 1]][1] && D_8010BFD8[SelectionPlayer][1][0].glyph != -1) {
                D_8010C0D0[SelectionPlayer & 1] = 0;
                MenuCursor = 0;
                for (i = 0; i < 4; i++) {
                    row = ovl667_D_80118E1C[SelectionPlayer & 1][D_8010C0D8[SelectionPlayer & 1]][i];
                    if (row == -1) {
                        break;
                    }
                    if (UiRows[row].byte14 == 2) {
                        UiRows[row].byte14 = 3;
                    }
                }
                for (i = 0; i < 4; i++) {
                    row = ovl667_D_80118E1C[SelectionPlayer & 1][(s16)(D_8010C0D8[SelectionPlayer & 1] ^ 1)][i];
                    if (row == -1) {
                        break;
                    }
                    UiRows[row].byte14 = 0;
                }
                D_800F4D8A = 7;
                D_8010C0D8[SelectionPlayer & 1] ^= 1;
            } else if (MenuCursor == ovl667_D_80118E14[D_8010C0D8[SelectionPlayer & 1]][2]) {
                cancel = 1;
            } else if (MenuCursor == ovl667_D_80118E14[D_8010C0D8[SelectionPlayer & 1]][3]) {
                for (i = 0; i < UiRowCount; i++) {
                    if (UiRows[i].byte14 == 2) {
                        UiRows[i].byte14 = 3;
                    }
                }
                ImageRows[1] = 3;
                MenuFadeTimer = 0x40;
                D_800F4D8A = 10;
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
            D_800F4D8A = 8;
            BadgeRows->state10 = 4;
            SeqFadeVolume(0, 0x3C);
        }
        break;
    case 3:
        row = ovl667_D_80118E1C[(SelectionPlayer - 1) & 1][1][3] + 1;
        if (UiRows[row].byte14 == 5) {
            D_8010C0B8 = 1;
            D_800F4D8A = 0;
        }
        break;
    case 4:
        row = ovl667_D_80118E1C[SelectionPlayer & 1][1][3] + 1;
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
                row = ovl667_D_80118E1C[SelectionPlayer & 1][D_8010C0D8[SelectionPlayer & 1]][i];
                if (row == -1) {
                    break;
                }
                if (UiRows[row].byte14 == 2) {
                    UiRows[row].byte14 = 3;
                }
            }
            modelReady = 0;
            MenuArenas.first = LoadCursor;
            if ((SelectionPlayer & 1) > 0 || LinkEntryState[(1 - D_800AD508) * 2 + 1] != 1) {
                D_800F4D8A = 5;
                for (i = 0; i < UiRowCount; i++) {
                    if (UiRows[i].byte14 == 2) {
                        UiRows[i].byte14 = 3;
                    }
                }
                ImageRows[1] = 3;
                BadgeRows->state10 = 4;
                break;
            }
            D_800F4D8A = 3;
            SelectionPlayer = SelectionPlayer + 1;
            ovl667_func_80116BB4();
        } else if (PlayerControllers[SelectionPlayer & 1].b != 0) {
            PlaySfx(2);
            UiRows[row].byte14 = 3;
            UiRows[row + 1].byte14 = 3;
            D_800F4D8A = 6;
        }
        break;
    case 5:
        SetMenuFrameOptions(0, 1, 1);
        row = ovl667_D_80118E1C[SelectionPlayer & 1][1][3] + 1;
        closing = UiRows[row].byte14;
        if (closing == 5 && (s8)ImageRows[1] == closing) {
            ResetScreenCounters();
            ovl667_func_80117EF8();
            ModeGroup = 1;
            ModeId = 61;
            D_800F4D8A = 0;
        }
        break;
    case 6:
        row = ovl667_D_80118E1C[SelectionPlayer & 1][1][3] + 1;
        if (UiRows[row].byte14 == 5) {
            D_800F4D8A = 0;
            D_8010C0B8 = 1;
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
            D_800F4D8A = 0;
        }
        break;
    case 10:
        SetModelTexturePage(-1);
        SetMenuFrameOptions(0, 1, 1);
        MenuFadeTimer = (u16)MenuFadeTimer - 1;
        AddScreenFadeTiles(&CurrentDrawBuf->ordering.ordering70[0], (0x40 - MenuFadeTimer) * 4, 0x45);
        if (MenuFadeTimer <= 0) {
            D_800FFC18 = 2;
            ModeGroup = 1;
            ModeId = 71;
            D_800F4D8A = 0;
        }
        break;
    }
draw_rows:
    if (D_8010C0B8 == 0 && (s8)ImageRows[1] == 2) {
        D_8010C0B8 = 1;
        BadgeRows->half02 = 1;
    }
    if (D_800F4D8A == 3) {
        player = (u16)SelectionPlayer - 1;
    } else {
        player = SelectionPlayer;
    }
    if (D_800F4D8A == 7) {
        page = D_8010C0D8[player & 1];
        other = page ^ 1;
        row = ovl667_D_80118E1C[player & 1][(s16)other][0];
        if (UiRows[row].byte14 == 5) {
            other = page;
        }
    } else {
        other = D_8010C0D8[player & 1];
    }
    for (i = 0; i < 4; i++) {
        row = ovl667_D_80118E1C[player & 1][(s16)other][i];
        if (row == -1) {
            break;
        }
        DrawUiRow(row);
    }
    if (D_8010C0B8 == 2) {
        row = ovl667_D_80118E1C[player & 1][1][3];
        DrawUiRow(row + 1);
        DrawUiRow(row + 2);
    }
    if ((u32)((u8)ImageRows[1] - 1) < 2U) {
        drawModel = modelReady;
        if (drawModel == 1) {
            DrawSelectionIconGrid(SelectionPlayer);
            func_8002E5A0(&PlayerCars[SelectionPlayer & 1]);
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
    D_800F4CB8[0] = SelectionPlayer + 1;
    UpdateScreenFade(D_8010C0D8[SelectionPlayer & 1] != 0 ? 0x10036 : 0x10032);
}
