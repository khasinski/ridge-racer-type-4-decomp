#include "rr/overlay659.h"

void ovl659_init_results(void) {
    s32 i;
    s32 offset;

    ovl659_rank_standings();
    InitMenuTextRows(D_800FFC48);
    MenuCursor = 0;
    MenuItemCount = 2;
    offset = 0;
    if (SelectionState.mode1A != 4) {
        offset = 11;
    }
    for (i = 0; i < 11; i++) {
        if (SelectionState.phase17 >= 4 && i == 4 && offset == 0) {
            SetMenuTextRow(i, i, 0, 0, ovl659_result_labels[22]);
        } else {
            SetMenuTextRow(i, i, 0, 0, ovl659_result_labels[i + offset]);
        }
    }
    for (i = 0; i < 11; i++) {
        SetMenuTextRow(i + 33, i, 2, 0, ovl659_result_labels[i + 23]);
    }
    SetMenuFrameOptions(0, 1, 0);
    ovl659_preview_timer = -2;
    ModeFrame = 0;
}

void ovl659_update_results(void) {
    s32 group;
    s32 lapOffset;
    s32 rowOffset;
    s32 i;
    s16 timer;
    s32 ready;

    ModeFrame = (s32)((u32)ModeFrame + 1U);
    switch (D_800F4D8A) {
    case 0:
        group = 0;
        if (MenuTextRows[3].signedBytes[3] >= 16) {
            SetMenuFrameOptions(1, 1, 0);
            if (MenuConfirmPressed != 0) {
                SetMenuFrameOptions(0, 1, 0);
                PlaySfx(1);
                if (SelectionState.mode1A == 4) {
                    InitMenuArenas();
                    ResetScreenCounters();
                    D_800F4D8A = 5;
                    break;
                } else {
                    D_800F4D8A = 3;
                }
            }
        }
        lapOffset = 0;
        if (CourseIndex == 7) {
            if (rsin((ModeFrame & 0xFF) << 4) > 0) {
                lapOffset = 3;
            }
        }
        rowOffset = 6;
        if (SelectionState.mode1A != 4) {
            rowOffset = 5;
        }
        for (i = 0; i < 3; i++) {
            if (i >= NumLaps) {
                MenuTextRows[rowOffset + 2].state.d34 = -1;
                break;
            }
            if (i + lapOffset == PlayerCars[0].steering288.finish20) {
                func_80041320(i + rowOffset, 1, -1);
            } else {
                func_80041320(i + rowOffset, 0, -1);
            }
            FormatDecimalDigits((char *)&MenuTextRows[i + rowOffset].bytes[32], i + lapOffset + 1, 0);
            FormatRaceTime((char *)&MenuTextRows[i + rowOffset].bytes[35], LapTimes[i + lapOffset]);
        }
        break;
    case 3:
        group = 1;
        if (SelectionState.retries08[SelectionState.phase17] == 0) {
            D_800F4D8A = 7;
        } else if (MenuTextRows[38].signedBytes[3] >= 14) {
            SetMenuFrameOptions(1, 1, 0);
            func_80041320(MenuCursor + 37, 0, 0);
            StepMenuValue(0, 2, (char *)&MenuCursor, MenuItemCount, 0);
            func_80041320(MenuCursor + 37, 1, 1);
            if (MenuConfirmPressed != 0) {
                SetMenuFrameOptions(0, 1, 0);
                PlaySfx(1);
                if (MenuCursor == 0) {
                    D_800F4D8A = 4;
                    MenuFadeTimer = 64;
                    SelectionState.retries08[SelectionState.phase17]--;
                    FormatDecimalDigits((char *)&MenuTextRows[40].bytes[36],
                                  SelectionState.retries08[SelectionState.phase17], 0);
                } else {
                    D_800F4D8A = 7;
                }
            }
        }
        break;
    case 4:
        timer = (u16)MenuFadeTimer - 1;
        MenuFadeTimer = timer;
        AddScreenFadeTiles(&CurrentDrawBuf->ordering.ordering70[0], (64 - timer) * 4, 69);
        group = 1;
        if (MenuFadeTimer <= 0) {
            MenuScriptId = 0;
            ModeGroup = 1;
            ModeId = 35;
            D_800F4D8A = 0;
        }
        break;
    case 5:
        group = 0;
        for (i = 0; i < 11; i++) {
            if (MenuTextRows[i].signedBytes[2] < 8) {
                MenuTextRows[i].signedBytes[2] = 8;
                MenuTextRows[i].signedBytes[0] = 0;
            }
        }
        if (MenuTextRows[0].signedBytes[2] == 9) {
            D_800F4D8A = 6;
            ovl659_preview_timer = 64;
        }
        break;
    case 6:
        SetMenuFrameOptions(0, 1, 0);
        group = -1;
        if (ovl659_preview_timer > 0) {
            ovl659_preview_timer--;
        }
        ready = LoadResultsSceneStep();
        if (ready == 1 && ovl659_preview_timer <= 0) {
            if (SelectionState.phase17 == 7) {
                UpdateSaveProgress();
                ModeGroup = 0;
                ModeId = 3;
            } else {
                ResetScreenCounters();
                SeqStartTrack(1);
                ModeGroup = ready;
                ModeId = 11;
            }
            D_800F4D8A = 0;
            UpdateScreenFade(0x1000C);
            return;
        }
        break;
    case 7:
        group = 0;
        for (i = 0; i < 11; i++) {
            if (MenuTextRows[i].signedBytes[2] < 8) {
                MenuTextRows[i].signedBytes[2] = 8;
                MenuTextRows[i].signedBytes[0] = 0;
            }
        }
        if (MenuTextRows[0].signedBytes[2] == 9) {
            D_800F4D8A = 8;
        }
        break;
    case 8:
        InitMenuArenas();
        ModeGroup = 1;
        ModeId = 31;
        D_800F4D8A = 0;
        UpdateScreenFade(0x1000C);
        return;
    default:
        break;
    }
    UpdateScreenFade(0x1000C);
    if (ovl659_preview_timer == -2) {
        func_8003662C(ModeFrame);
    } else {
        func_8003662C(ovl659_preview_timer);
    }
    if (group >= 0) {
        i = 0;
        do {
            UpdateMenuTextRow(i + group * 33);
            i++;
        } while (i < 11);
    }
    if (MenuTextRows[0].signedBytes[2] >= 3 && D_800F4D8A < 6) {
        func_800411D4();
    }
}

void ovl659_rank_standings(void) {
    s32 values[6];
    u16 indices[6];
    s32 n, rank;
    s32 *destination, *source;
    u16 *order;
    s32 i;
    s32 j, previous;
    s32 *left, *right;
    s32 l, r;
    u16 *slot, *prior;
    u16 index;
    RrRaceVehicle *vehicle;
    n = 6;
    i = 0;
    destination = values;
    source = LapTimes;
    order = indices;
    for (; i < 6; destination++, source++, i++, order++) {
        *order = i;
        *destination = *source;
        if (*destination < 0) {
            n = i;
            break;
        }
    }
    for (i = 0; i < n - 1; i++) {
        for (j = n - 1; j > i; j = previous) {
            previous = j - 1;
            left = &values[previous];
            right = &values[j];
            l = *left;
            r = *right;
            if (r < l) {
                slot = &indices[j];
                index = *slot;
                *right = l;
                prior = &indices[previous];
                *left = r;
                *slot = *prior;
                *prior = index;
            }
        }
    }
    rank = TotalRaceTime;
    if (rank == -1) {
        vehicle = PlayerCars;
        vehicle->steering288.finish20 = rank;
        if (vehicle->steering288.count22 > 0)
            LapTimes[vehicle->steering288.count22 - 1] = rank;
    } else {
        D_800AC1A0 = indices[0];
    }
}
