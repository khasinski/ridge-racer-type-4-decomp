#include "rr/overlay660.h"

void ovl660_func_801154D8(void)
{
    s32 i;
    s32 off;
    s32 mode;

    ModeFrame = 0;
    if (TotalRaceTime != -1) {
        ovl660_func_801162D8();
        ovl660_race_record_rank = ovl660_func_80115FA8(0, LapTimes[(s16)D_800AC1A0]);
        ovl660_lap_record_rank = ovl660_func_80115FA8(1, TotalRaceTime);
    } else {
        ovl660_race_record_rank = 6;
        ovl660_lap_record_rank = 6;
    }
    InitMenuTextRows(D_800FFC48);
    MenuCursor = 0;
    MenuItemCount = 3;
    off = 0;
    if (CourseIndex == 7) {
        off = 11;
    }
    for (i = 0; i < 44; i++) {
        SetMenuTextRow(i, i % 11, (i >= 11) * 2, 0, ovl660_result_labels[off + i]);
        if (i == 10) {
            off = 11;
        }
    }
    for (i = 44; i < 55; i++) {
        SetMenuTextRow(i, i - 44, 0, 0, -1);
    }
    ovl660_result_kind = 0;
    mode = D_800F3A48;
    if (mode == 2) {
        ovl660_result_kind = mode;
    } else if (mode == 1) {
        ovl660_result_kind = mode;
    }
    ovl660_result_transition = 0;
    SetMenuFrameOptions(0, 1, 0);
}

s32 ovl660_func_801156A8(s32 reserved){
 func_80041320(MenuCursor+37,0,0);
 StepMenuValue(0,2,(char*)&MenuCursor,MenuItemCount,0);
 func_80041320(MenuCursor+37,1,1);
 if(MenuConfirmPressed){PlaySfx(1);return MenuCursor;}
 return -1;
}

void ovl660_func_8011572C(void) {
    s32 group;
    s32 base;
    s32 row;
    s32 i;
    s16 timer;
    s32 ready;

    ModeFrame++;
    switch (D_800F4D8A) {
    case 0:
        if (TotalRaceTime == -1) {
            group = 44;
            if (MenuTextRows[44].signedBytes[2] >= 2) {
                D_800F4D8A = 3;
            }
            break;
        }
        group = 0;
        if (MenuTextRows[1].signedBytes[3] >= 15) {
            SetMenuFrameOptions(1, 1, 0);
            if (MenuConfirmPressed != 0) {
                PlaySfx(1);
                ovl660_name_cursor = 0;
                D_800F4D8A++;
                SetMenuFrameOptions(0, 1, 0);
            }
        }
        base = 5;
        if (CourseIndex == 7) {
            base = 3;
        }
        for (i = 0; i < NumLaps; i++) {
            row = base + i;
            FormatDecimalDigits((char *)&MenuTextRows[row].bytes[32], i + 1, 0);
            FormatRaceTime((char *)&MenuTextRows[row].bytes[35], LapTimes[i]);
            if (i == PlayerCars[0].steering288.finish20 && ovl660_race_record_rank < 5) {
                func_80041320(row, 1, -1);
                CopyStringClearTail((char *)&MenuTextRows[row].bytes[45], ovl660_new_record_label);
            }
        }
        base = 9;
        if (CourseIndex == 7) {
            base = 10;
        }
        if (ovl660_lap_record_rank < 5) {
            func_80041320(base, 1, -1);
            CopyStringClearTail((char *)&MenuTextRows[base].bytes[45], ovl660_new_record_label);
        }
        break;
    case 1:
        group = 11;
        if (ovl660_race_record_rank < 5) {
            StepMenuValue(1, 4, (char *)&ovl660_name_cursor, 8, 0);
            StepNameChar(&ovl660_record_name[ovl660_name_cursor]);
            {
                s32 entry = ovl660_race_record_rank + 15;
                CopyStringClearTail((char *)&MenuTextRows[entry].bytes[42], ovl660_record_name);
            }
            CopyStringClearTail(LapRecords[ReverseCourse][CourseIndex][ovl660_race_record_rank].name, ovl660_record_name);
            func_80041320(ovl660_race_record_rank + 15, 1, -((ovl660_name_cursor + 15) << 8));
        }
        if (MenuTextRows[15].signedBytes[3] >= 15) {
            SetMenuFrameOptions(1, 1, 0);
            if (MenuConfirmPressed != 0) {
                PlaySfx(1);
                ovl660_name_cursor = 0;
                D_800F4D8A++;
                SetMenuFrameOptions(0, 1, 0);
            }
        }
        break;
    case 2:
        group = 22;
        if (ovl660_lap_record_rank < 5) {
            StepMenuValue(1, 4, (char *)&ovl660_name_cursor, 8, 0);
            StepNameChar(&ovl660_record_name[ovl660_name_cursor]);
            {
                s32 entry = ovl660_lap_record_rank + 26;
                CopyStringClearTail((char *)&MenuTextRows[entry].bytes[42], ovl660_record_name);
            }
            CopyStringClearTail(RaceRecords[ReverseCourse][CourseIndex][ovl660_lap_record_rank].name, ovl660_record_name);
            func_80041320(ovl660_lap_record_rank + 26, 1, -((ovl660_name_cursor + 15) << 8));
        }
        if (MenuTextRows[15].signedBytes[3] >= 15) {
            SetMenuFrameOptions(1, 1, 0);
            if (MenuConfirmPressed != 0) {
                PlaySfx(1);
                D_800F4D8A++;
                SetMenuFrameOptions(0, 1, 0);
            }
        }
        break;
    case 3:
        group = 33;
        if (MenuTextRows[38].signedBytes[3] >= 17) {
            SetMenuFrameOptions(1, 1, 0);
            if (ovl660_func_801156A8(0) >= 0) {
                SetMenuFrameOptions(0, 1, 0);
                ResetScreenCounters();
                InitMenuArenas();
                MenuFadeTimer = 64;
                D_800F4D8A = MenuCursor + 4;
            }
        } else {
            SetMenuFrameOptions(0, 1, 0);
        }
        if (TotalRaceTime != -1) {
            if (ovl660_result_transition == 1) {
                ready = PackChoiceGlyph((s8 *)&SelectionRows);
                MenuTextRows[42].bytes[28] = 34;
                EncodeRecordPassword(CourseMenuSelection, ready, TotalRaceTime, ovl660_result_kind, D_800AD564,
                              &MenuTextRows[42].bytes[29]);
                MenuTextRows[42].bytes[42] = 34;
                ovl660_result_transition = 2;
            } else if (ovl660_result_transition == 0 && (*(s32 *)&PlayerControllers[0] & 0x900000) == 0x900000) {
                ovl660_result_transition = 1;
            }
        }
        break;
    case 4:
        timer = (u16)MenuFadeTimer - 1;
        MenuFadeTimer = timer;
        AddScreenFadeTiles(&CurrentDrawBuf->ordering.ordering70[0], (64 - timer) * 4, 69);
        group = 33;
        if (MenuFadeTimer <= 0) {
            MenuScriptId = 0;
            ModeGroup = 1;
            ModeId = 35;
            D_800F4D8A = 0;
        }
        break;
    case 5:
        timer = (u16)MenuFadeTimer - 2;
        MenuFadeTimer = timer;
        AddScreenFadeTiles(&CurrentDrawBuf->ordering.ordering70[0], (64 - timer) * 4, 69);
        group = 33;
        if (MenuFadeTimer <= 0) {
            group = -1;
            D_800F4D8A = 8;
        }
        break;
    case 6:
        timer = (u16)MenuFadeTimer - 1;
        MenuFadeTimer = timer;
        AddScreenFadeTiles(&CurrentDrawBuf->ordering.ordering70[0], (64 - timer) * 4, 69);
        group = -1;
        if (MenuFadeTimer <= 0) {
            ModeGroup = 1;
            ModeId = 3;
            D_800F4D8A = 0;
        }
        break;
    case 8:
        ready = LoadResultsSceneStep();
        if (ready == 1) {
            SeqStartTrack(0);
            ovl660_func_80114920();
            ModeGroup = ready;
            ModeId = 50;
            D_800F4D8A = 0;
        }
        return;
    }
    UpdateScreenFade(0x1000C);
    func_8003662C(ModeFrame);
    if (group >= 0) {
        i = 0;
        do {
            UpdateMenuTextRow(i + group);
            i++;
        } while (i < 11);
    }
    if ((u32)(MenuTextRows[0].bytes[2] - 3) < 5 || (u32)(MenuTextRows[44].bytes[2] - 3) < 5) {
        func_800411D4();
    }
}

s32 ovl660_func_80115FA8(s32 mode, s32 time) {
    s32 times[6];
    s16 order[6];
    s32 i;
    s32 j;
    s16 o;

    times[5] = time;
    order[5] = 5;
    for (i = 0; i < 5; i++) {
        order[i] = i;
        if (mode == 1) {
            times[i] = RaceRecords[ReverseCourse][CourseIndex][i].time;
        } else {
            times[i] = LapRecords[ReverseCourse][CourseIndex][i].time;
        }
    }
    for (i = 0; i < 5; i++) {
        for (j = 5; i < j; j--) {
            s32 earlier = times[j - 1];
            s32 later = times[j];
            if (later < earlier) {
                o = order[j];
                times[j] = earlier;
                times[j - 1] = later;
                order[j] = order[j - 1];
                order[j - 1] = o;
            }
        }
    }
    for (i = 0; i < 5; i++) {
        if (order[i] == 5) {
            for (j = 4; i < j; j--) {
                if (mode == 1) {
                    RaceRecords[ReverseCourse][CourseIndex][j] = RaceRecords[ReverseCourse][CourseIndex][j - 1];
                } else {
                    LapRecords[ReverseCourse][CourseIndex][j] = LapRecords[ReverseCourse][CourseIndex][j - 1];
                }
            }
            if (mode == 1) {
                RaceRecords[ReverseCourse][CourseIndex][i].car = PackChoiceGlyph((s8 *)&SelectionRows);
                RaceRecords[ReverseCourse][CourseIndex][i].time = times[i];
                CopyStringClearTail(RaceRecords[ReverseCourse][CourseIndex][i].name, ovl660_record_name);
            } else {
                LapRecords[ReverseCourse][CourseIndex][i].car = PackChoiceGlyph((s8 *)&SelectionRows);
                LapRecords[ReverseCourse][CourseIndex][i].time = times[i];
                CopyStringClearTail(LapRecords[ReverseCourse][CourseIndex][i].name, ovl660_record_name);
            }
            return i;
        }
    }
    return i;
}

void ovl660_func_801162D8(void)
{
    s32 values[6];
    u16 indices[6];
    s32 n;
    s32 *destination, *source;
    u16 *order;
    s32 i;
    s32 j, previous;
    s32 *left, *right;
    s32 l, r;
    u16 *slot, *prior;
    u16 index;
    n = 6;
    i = 0;
    destination = values; source = LapTimes; order = indices;
    for (; i < 6; destination++, source++, i++, order++) {
        *order = i;
        *destination = *source;
        if (*destination < 0) { n = i; break; }
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
    D_800AC1A0 = indices[0];
}
