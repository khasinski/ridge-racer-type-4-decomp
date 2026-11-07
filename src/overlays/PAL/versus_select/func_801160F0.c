#include "rr/menu_overlay_runtime.h"

/* Versus course selection: each player picks a course group and course. */
extern u8 versus_select_group_option_counts[];            /* option count per course group */
extern s8 versus_select_group_option_ids[2][4];        /* option meaning per group slot */
extern s8 versus_select_group_option_rows[2][2][4];     /* UI row per player, group, slot */
extern s16 versus_select_subchoice_state;
extern s16 versus_select_choice_counts[2][2];       /* course count per player/group */
extern s16 versus_select_choice_index[2];          /* selected course per player */
extern s16 versus_select_choice_group[2];          /* selected group per player */
extern RrCourseChoice4 versus_select_choice_tables[2][2][9];
extern s32 UiRowCount, MenuActivePlayerPlusOne;
extern s16 MenuResultCode;
void SetupMenuCarCamera(s32);
void SetupViewport(s32);
s32 LoadCarRangeStep(s32, s32);
void SetCarBounds(RrRaceVehicle *);
void PrepareCarDrawPose(RrRaceVehicle *);
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
void versus_select_init_selection_screen(s32);
void versus_select_apply_player_choice(void);
void versus_select_init_course_select(void);

#define MODE (((s8 *)ImageRows)[1])
#define SELECTION ((s8 *)&SelectionRows)
#define ENTRIES ((RrSelectionRow *)&SelectionRows)
#define PLAYER (SelectionPlayer & 1)
#define GROUP_ROW(p, g, i) versus_select_group_option_rows[p][g][i]

void versus_select_update_car_select(void)
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
    switch (ModeSubState) {
    case 7:
        row = GROUP_ROW(PLAYER, (s16)(versus_select_choice_group[PLAYER] ^ 1), 0);
        if (UiRows[row].byte14 == 5) {
            ModeSubState = 0;
        }
    case 0:
        SetMenuFrameOptions(1, 1, 1);
        before = PackChoiceGlyph(SELECTION + SelectionPlayer * 4);
        StepMenuValue(PLAYER * 4 | 1, 2, (char *)&versus_select_choice_index[PLAYER],
                      versus_select_choice_counts[PLAYER][versus_select_choice_group[PLAYER]], 0);
        UnpackChoiceGlyph(SELECTION + SelectionPlayer * 4,
                      versus_select_choice_tables[PLAYER][versus_select_choice_group[PLAYER]][versus_select_choice_index[PLAYER]].glyph);
        if ((s16)before != (s16)PackChoiceGlyph(SELECTION + SelectionPlayer * 4)) {
            ResetScreenCounters();
            LoadCursor = MenuArenas.first;
            UnpackChoiceGlyph(SELECTION + SelectionPlayer * 4,
                          versus_select_choice_tables[PLAYER][versus_select_choice_group[PLAYER]][versus_select_choice_index[PLAYER]].glyph);
            PlayerCourseChoices[SelectionPlayer].first = versus_select_choice_tables[PLAYER][versus_select_choice_group[PLAYER]][versus_select_choice_index[PLAYER]].first;
            PlayerCourseChoices[SelectionPlayer].second = versus_select_choice_tables[PLAYER][versus_select_choice_group[PLAYER]][versus_select_choice_index[PLAYER]].second;
            MODE = 6;
        }
        row = GROUP_ROW(PLAYER, versus_select_choice_group[PLAYER], MenuCursor);
        UiRows[row].cells[0] = 0;
        StepMenuValue(PLAYER * 4, 2, (char *)&MenuCursor, versus_select_group_option_counts[versus_select_choice_group[PLAYER]], 0);
        row = GROUP_ROW(PLAYER, versus_select_choice_group[PLAYER], MenuCursor);
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
            if (MenuCursor == versus_select_group_option_ids[versus_select_choice_group[PLAYER]][0]) {
                versus_select_subchoice_state = i;
                ModeSubState = 4;
            } else if (MenuCursor == versus_select_group_option_ids[versus_select_choice_group[PLAYER]][1]
                       && versus_select_choice_tables[SelectionPlayer][1][0].glyph != -1) {
                versus_select_choice_index[PLAYER] = 0;
                MenuCursor = 0;
                for (i = 0; i < 4; i++) {
                    row = GROUP_ROW(PLAYER, versus_select_choice_group[PLAYER], i);
                    if (row == -1) {
                        break;
                    }
                    if (UiRows[row].byte14 == 2) {
                        UiRows[row].byte14 = 3;
                    }
                }
                for (i = 0; i < 4;) {
                    row = GROUP_ROW(PLAYER, (s16)(versus_select_choice_group[PLAYER] ^ 1), i);
                    i++;
                    if (row == -1) {
                        break;
                    }
                    UiRows[row].byte14 = 0;
                }
                ModeSubState = 7;
                versus_select_choice_group[PLAYER] ^= 1;
            } else if (MenuCursor == versus_select_group_option_ids[versus_select_choice_group[PLAYER]][2]) {
                close = 1;
            } else if (MenuCursor == versus_select_group_option_ids[versus_select_choice_group[PLAYER]][3]) {
                for (i = 0; i < UiRowCount; i++) {
                    if (UiRows[i].byte14 == 2) {
                        UiRows[i].byte14 = 3;
                    }
                }
                MODE = 3;
                MenuFadeTimer = 0x40;
                ModeSubState = 10;
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
            ModeSubState = 8;
        }
        break;
    case 3:
        row = GROUP_ROW((SelectionPlayer - 1) & 1, 1, 3) + 1;
        if (UiRows[row].byte14 == 5) {
            versus_select_subchoice_state = 1;
            ModeSubState = 0;
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
                row = GROUP_ROW(PLAYER, versus_select_choice_group[PLAYER], i);
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
                ModeSubState = 5;
                for (i = 0; i < UiRowCount; i++) {
                    if (UiRows[i].byte14 == 2) {
                        UiRows[i].byte14 = 3;
                    }
                }
                MODE = 3;
                BadgeRows->state10 = 4;
            } else {
                ModeSubState = 3;
                SelectionPlayer++;
                versus_select_apply_player_choice();
            }
        } else if (input->b != 0) {
            PlaySfx(2);
            UiRows[row].byte14 = 3;
            UiRows[row + 1].byte14 = 3;
            ModeSubState = 6;
        }
        break;
    case 5:
        SetMenuFrameOptions(0, 1, 1);
        row = GROUP_ROW(PLAYER, 1, 3) + 1;
        status = UiRows[row].byte14;
        if (status == 5 && MODE == status) {
            ResetScreenCounters();
            versus_select_init_course_select();
            ModeGroup = 1;
            ModeId = 0x39;
            ModeSubState = 0;
        }
        break;
    case 6:
        row = GROUP_ROW(PLAYER, 1, 3) + 1;
        if (UiRows[row].byte14 == 5) {
            ModeSubState = 0;
            versus_select_subchoice_state = 1;
            UiRows[row].byte14 = 0;
            UiRows[row + 1].byte14 = 0;
        }
        break;
    case 9:
        row = GROUP_ROW(PLAYER, 1, 3) + 1;
        status = UiRows[row].byte14;
        if (status == 5 && MODE == status) {
            versus_select_init_selection_screen(0);
            ModeSubState = 0;
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
            ModeSubState = 0;
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
            ModeSubState = 0;
        }
        break;
    }
tail:
    if (versus_select_subchoice_state == 0 && MODE == 2) {
        versus_select_subchoice_state = 1;
        BadgeRows->half02 = 1;
    }
    if (ModeSubState == 3) {
        player = SelectionPlayer - 1;
    } else {
        player = SelectionPlayer;
    }
    if (ModeSubState == 7) {
        group = versus_select_choice_group[player & 1] ^ 1;
        row = GROUP_ROW(player & 1, group, 0);
        if (UiRows[row].byte14 == 5) {
            group = versus_select_choice_group[player & 1];
        }
    } else {
        group = versus_select_choice_group[player & 1];
    }
    for (i = 0; i < 4; i++) {
        row = GROUP_ROW(player & 1, group, i);
        if (row == -1) {
            break;
        }
        DrawUiRow(row);
    }
    if (versus_select_subchoice_state == 2) {
        last = GROUP_ROW(player & 1, 1, 3);
        DrawUiRow(last + 1);
        DrawUiRow(last + 2);
    }
    if ((u8)MODE - 1 < 2U) {
        i = active;
        if (i == 1) {
            DrawSelectionIconGrid(SelectionPlayer);
            PrepareCarDrawPose(&PlayerCars[PLAYER]);
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
        DrawMenuStepArrows(0xA0, 0x90, 8, 1, versus_select_choice_index[PLAYER], 0,
                      versus_select_choice_counts[PLAYER][versus_select_choice_group[PLAYER]]);
    }
    MenuActivePlayerPlusOne = SelectionPlayer + 1;
    sound = 0x10032;
    /* Integer base address: the original's table read is not disambiguated
     * from the MenuActivePlayerPlusOne store above. */
    groups = (s32)versus_select_choice_group;
    if (*(s16 *)(groups + (SelectionPlayer & 1) * 2) != 0) {
        sound = 0x10036;
    }
    UpdateScreenFade(sound);
}
