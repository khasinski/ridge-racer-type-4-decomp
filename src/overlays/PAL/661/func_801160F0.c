#include "rr/menu_overlay_runtime.h"

/* Versus course selection: each player picks a course group and course. */
extern u8 ovl661_D_80118358[];            /* option count per course group */
extern s8 ovl661_D_8011835C[2][4];        /* option meaning per group slot */
extern s8 ovl661_D_80118364[2][2][4];     /* UI row per player, group, slot */
extern s16 ovl661_D_80118438;
extern s16 ovl661_D_80118440[2][2];       /* course count per player/group */
extern s16 ovl661_D_8011844C[2];          /* selected course per player */
extern s16 ovl661_D_80118450[2];          /* selected group per player */
extern RrCourseChoice4 ovl661_D_80118458[2][2][9];
extern s32 UiRowCount, MenuActivePlayerPlusOne;
extern s16 MenuResultCode;
void SetupMenuCarCamera(s32);
void SetupViewport(s32);
s32 LoadCarRangeStep(s32, s32);
void SetCarBounds(RrRaceVehicle *);
void func_8002E5A0(RrRaceVehicle *);
void DrawPreviewCar(RrRaceVehicle *, RrPreviewRgb3, s32, s32);
void SetMenuFrameOptions(s32, s32, s32);
void StepMenuValue(s32, s32, char *, s32, s32);
void UnpackChoiceGlyph(s8 *, s32);
s32 PackChoiceGlyph(s8 *);
void ResetScreenCounters(void);
void SetModelTexturePage(s32);
void PlaySfx(s32);
void DrawUiRow(s32);
void DrawSelectionIconGrid(s32);
void UpdateBadgeRowAnim(s32);
s32 UpdateImageRow(s32);
void DrawMenuStepArrows(s32, s32, s32, s32, s32, s32, s32);
void UpdateScreenFade(s32);
void AddScreenFadeTiles(u32 *, s32, u32);
void SeqFadeVolume(s32, s32);
void SeqMuteVolume(void);
void ovl661_func_80115598(s32);
void ovl661_func_80115F8C(void);
void ovl661_func_80117350(void);

#define MODE (((s8 *)ImageRows)[1])
#define SELECTION ((s8 *)&SelectionRows)
#define ENTRIES ((RrSelectionRow *)&SelectionRows)
#define PLAYER (SelectionPlayer & 1)
#define GROUP_ROW(p, g, i) ovl661_D_80118364[p][g][i]

void ovl661_func_801160F0(void)
{
    union { RrPreviewRgb3 rgb; u8 channel[3]; } color;
    s16 active;
    s32 result;
    s32 row;
    u8 status;
    s32 before;
    s32 i;
    s32 close;
    s16 player;
    s16 group;
    s32 brightness;
    s32 flags;
    s32 sound;
    s32 groups;
    s8 last;
    RrRaceVehicle *vehicle;
    RrRaceVehicle *vehicles;
    s32 *tick;
    RrMenuItemState5C *input;

    active = 0;
    tick = &ModeFrame;
    vehicles = PlayerCars;
    vehicle = &vehicles[PLAYER];
    (*tick)++;
    vehicle->rot54 = (vehicle->rot54 + 8) & 0xFFF;
    SetupMenuCarCamera(1);
    SetupViewport(6);
    result = LoadCarRangeStep(SelectionPlayer, 1);
    if (result != 1) {
        if (MODE == 1) {
            MODE = 0;
        }
        goto tail;
    }
    SetCarBounds(&vehicles[PLAYER]);
    if (MODE == 7) {
        MODE = result;
    }
    active = 1;
    switch (D_800F4D8A) {
    case 7:
        row = GROUP_ROW(PLAYER, (s16)(ovl661_D_80118450[PLAYER] ^ 1), 0);
        if (UiRows[row].byte14 == 5) {
            D_800F4D8A = 0;
        }
    case 0:
        SetMenuFrameOptions(1, 1, 1);
        before = PackChoiceGlyph(SELECTION + SelectionPlayer * 4);
        StepMenuValue(PLAYER * 4 | 1, 2, (char *)&ovl661_D_8011844C[PLAYER],
                      ovl661_D_80118440[PLAYER][ovl661_D_80118450[PLAYER]], 0);
        UnpackChoiceGlyph(SELECTION + SelectionPlayer * 4,
                      ovl661_D_80118458[PLAYER][ovl661_D_80118450[PLAYER]][ovl661_D_8011844C[PLAYER]].glyph);
        if ((s16)before != (s16)PackChoiceGlyph(SELECTION + SelectionPlayer * 4)) {
            ResetScreenCounters();
            LoadCursor = MenuArenas.first;
            UnpackChoiceGlyph(SELECTION + SelectionPlayer * 4,
                          ovl661_D_80118458[PLAYER][ovl661_D_80118450[PLAYER]][ovl661_D_8011844C[PLAYER]].glyph);
            PlayerCourseChoices[SelectionPlayer].first = ovl661_D_80118458[PLAYER][ovl661_D_80118450[PLAYER]][ovl661_D_8011844C[PLAYER]].first;
            PlayerCourseChoices[SelectionPlayer].second = ovl661_D_80118458[PLAYER][ovl661_D_80118450[PLAYER]][ovl661_D_8011844C[PLAYER]].second;
            MODE = 6;
        }
        row = GROUP_ROW(PLAYER, ovl661_D_80118450[PLAYER], MenuCursor);
        UiRows[row].cells[0] = 0;
        StepMenuValue(PLAYER * 4, 2, (char *)&MenuCursor, ovl661_D_80118358[ovl661_D_80118450[PLAYER]], 0);
        row = GROUP_ROW(PLAYER, ovl661_D_80118450[PLAYER], MenuCursor);
        UiRows[row].cells[0] = 2;
        i = UiRows[row].byte14;
        close = 0;
        if (i != 2) {
            break;
        }
        if (BadgeRows->state10 != 3) {
            break;
        }
        {
            RrMenuItemState5C *inputs = PlayerControllers;
            input = inputs + PLAYER;
        }
        if (input->a != 0) {
            PlaySfx(1);
            UiRows[row].cells[0] = 1;
            if (MenuCursor == ovl661_D_8011835C[ovl661_D_80118450[PLAYER]][0]) {
                ovl661_D_80118438 = i;
                D_800F4D8A = 4;
            } else if (MenuCursor == ovl661_D_8011835C[ovl661_D_80118450[PLAYER]][1]
                       && ovl661_D_80118458[SelectionPlayer][1][0].glyph != -1) {
                ovl661_D_8011844C[PLAYER] = 0;
                MenuCursor = 0;
                for (i = 0; i < 4; i++) {
                    row = GROUP_ROW(PLAYER, ovl661_D_80118450[PLAYER], i);
                    if (row == -1) {
                        break;
                    }
                    if (UiRows[row].byte14 == 2) {
                        UiRows[row].byte14 = 3;
                    }
                }
                for (i = 0; i < 4;) {
                    row = GROUP_ROW(PLAYER, (s16)(ovl661_D_80118450[PLAYER] ^ 1), i);
                    i++;
                    if (row == -1) {
                        break;
                    }
                    UiRows[row].byte14 = 0;
                }
                D_800F4D8A = 7;
                ovl661_D_80118450[PLAYER] ^= 1;
            } else if (MenuCursor == ovl661_D_8011835C[ovl661_D_80118450[PLAYER]][2]) {
                close = 1;
            } else if (MenuCursor == ovl661_D_8011835C[ovl661_D_80118450[PLAYER]][3]) {
                for (i = 0; i < UiRowCount; i++) {
                    if (UiRows[i].byte14 == 2) {
                        UiRows[i].byte14 = 3;
                    }
                }
                MODE = 3;
                MenuFadeTimer = 0x40;
                D_800F4D8A = 10;
                BadgeRows->state10 = 4;
            }
        } else if (input->b != 0) {
            PlaySfx(2);
            close = 1;
        }
        if (close == 1) {
            for (i = 0; i < UiRowCount; i++) {
                if (UiRows[i].byte14 == 2) {
                    UiRows[i].byte14 = 3;
                }
            }
            MODE = 3;
            MenuFadeTimer = 0x40;
            BadgeRows->state10 = 4;
            SeqFadeVolume(0, 60);
            D_800F4D8A = 8;
        }
        break;
    case 3:
        row = GROUP_ROW((SelectionPlayer - 1) & 1, 1, 3) + 1;
        if (UiRows[row].byte14 == 5) {
            ovl661_D_80118438 = 1;
            D_800F4D8A = 0;
        }
        break;
    case 4:
        row = GROUP_ROW(PLAYER, 1, 3) + 1;
        i = UiRows[row].byte14;
        if (i != 2) {
            break;
        }
        UiRows[row + ENTRIES[SelectionPlayer].reserved].cells[0] = 0;
        StepMenuValue(PLAYER * 4, 1, (char *)&ENTRIES[SelectionPlayer].reserved, 2, 0);
        UiRows[row + ENTRIES[SelectionPlayer].reserved].cells[0] = i;
        {
            RrMenuItemState5C *inputs = PlayerControllers;
            input = inputs + PLAYER;
        }
        if (input->a != 0) {
            PlaySfx(1);
            UiRows[row + ENTRIES[0].reserved].cells[0] = 1;
            UiRows[row].byte14 = 3;
            UiRows[row + 1].byte14 = 3;
            for (i = 0; i < 4; i++) {
                row = GROUP_ROW(PLAYER, ovl661_D_80118450[PLAYER], i);
                if (row == -1) {
                    break;
                }
                if (UiRows[row].byte14 == 2) {
                    UiRows[row].byte14 = 3;
                }
            }
            active = 0;
            MenuArenas.first = LoadCursor;
            if ((SelectionPlayer & 1) > 0) {
                D_800F4D8A = 5;
                for (i = 0; i < UiRowCount; i++) {
                    if (UiRows[i].byte14 == 2) {
                        UiRows[i].byte14 = 3;
                    }
                }
                MODE = 3;
                BadgeRows->state10 = 4;
            } else {
                D_800F4D8A = 3;
                SelectionPlayer++;
                ovl661_func_80115F8C();
            }
        } else if (input->b != 0) {
            PlaySfx(2);
            UiRows[row].byte14 = 3;
            UiRows[row + 1].byte14 = 3;
            D_800F4D8A = 6;
        }
        break;
    case 5:
        SetMenuFrameOptions(0, 1, 1);
        row = GROUP_ROW(PLAYER, 1, 3) + 1;
        status = UiRows[row].byte14;
        if (status == 5 && MODE == status) {
            ResetScreenCounters();
            ovl661_func_80117350();
            ModeGroup = 1;
            ModeId = 0x39;
            D_800F4D8A = 0;
        }
        break;
    case 6:
        row = GROUP_ROW(PLAYER, 1, 3) + 1;
        if (UiRows[row].byte14 == 5) {
            D_800F4D8A = 0;
            ovl661_D_80118438 = 1;
            UiRows[row].byte14 = 0;
            UiRows[row + 1].byte14 = 0;
        }
        break;
    case 9:
        row = GROUP_ROW(PLAYER, 1, 3) + 1;
        status = UiRows[row].byte14;
        if (status == 5 && MODE == status) {
            ovl661_func_80115598(0);
            D_800F4D8A = 0;
        }
        break;
    case 8:
        SetMenuFrameOptions(0, 1, 1);
        MenuFadeTimer--;
        SetModelTexturePage(-1);
        AddScreenFadeTiles(CurrentDrawBuf->ordering.ordering70, (0x40 - MenuFadeTimer) * 4, 0x45);
        if (MenuFadeTimer <= 0) {
            SeqMuteVolume();
            ModeGroup = 1;
            ModeId = 3;
            D_800F4D8A = 0;
        }
        break;
    case 10:
        SetMenuFrameOptions(0, 1, 1);
        SetModelTexturePage(-1);
        MenuFadeTimer--;
        AddScreenFadeTiles(CurrentDrawBuf->ordering.ordering70, (0x40 - MenuFadeTimer) * 4, 0x45);
        if (MenuFadeTimer <= 0) {
            MenuResultCode = 2;
            ModeGroup = 1;
            ModeId = 0x47;
            D_800F4D8A = 0;
        }
        break;
    }
tail:
    if (ovl661_D_80118438 == 0 && MODE == 2) {
        ovl661_D_80118438 = 1;
        BadgeRows->half02 = 1;
    }
    if (D_800F4D8A == 3) {
        player = SelectionPlayer - 1;
    } else {
        player = SelectionPlayer;
    }
    if (D_800F4D8A == 7) {
        group = ovl661_D_80118450[player & 1] ^ 1;
        row = GROUP_ROW(player & 1, group, 0);
        if (UiRows[row].byte14 == 5) {
            group = ovl661_D_80118450[player & 1];
        }
    } else {
        group = ovl661_D_80118450[player & 1];
    }
    for (i = 0; i < 4; i++) {
        row = GROUP_ROW(player & 1, group, i);
        if (row == -1) {
            break;
        }
        DrawUiRow(row);
    }
    if (ovl661_D_80118438 == 2) {
        last = GROUP_ROW(player & 1, 1, 3);
        DrawUiRow(last + 1);
        DrawUiRow(last + 2);
    }
    if ((u8)MODE - 1 < 2U) {
        i = active;
        if (i == 1) {
            DrawSelectionIconGrid(SelectionPlayer);
            func_8002E5A0(&PlayerCars[PLAYER]);
            brightness = 0x100;
            if (MODE == i) {
                last = ~((ImageRows[2] * 0xFF) / 30);
                color.channel[0] = last;
                color.channel[1] = last;
                color.channel[2] = last;
                flags = 0x102;
                brightness = (ImageRows[2] << 8) / 30;
            } else {
                flags = 1;
                color.channel[0] = 0x80;
                color.channel[1] = 0x80;
                color.channel[2] = 0x80;
            }
            DrawPreviewCar(&PlayerCars[PLAYER], color.rgb, brightness, flags);
        }
    }
    UpdateBadgeRowAnim(0);
    if (UpdateImageRow(0) > 0) {
        DrawMenuStepArrows(0xA0, 0x90, 8, 1, ovl661_D_8011844C[PLAYER], 0,
                      ovl661_D_80118440[PLAYER][ovl661_D_80118450[PLAYER]]);
    }
    MenuActivePlayerPlusOne = SelectionPlayer + 1;
    sound = 0x10032;
    /* Integer base address: the original's table read is not disambiguated
     * from the MenuActivePlayerPlusOne store above. */
    groups = (s32)ovl661_D_80118450;
    if (*(s16 *)(groups + (SelectionPlayer & 1) * 2) != 0) {
        sound = 0x10036;
    }
    UpdateScreenFade(sound);
}
