/* RODATA 0x80010e20 0x7c */
#include "rr/menu_text_rows.h"
#include "rr/menu_state_types.h"
#include "rr/time_record_types.h"
#include "rr/string_tail_copy.h"
#include "rr/menu_choice.h"

void FormatDecimalDigits(char *output, s32 value, s32 digits);
void FormatRaceTime(char *output, s32 time);
extern char StrStarSpace[];
extern char LapTimeTemplateStr[];
extern u8 *MenuTextTemplates[];
extern char *OrdinalSuffixStrs[];
extern char *StrRetirePtr;
extern s8 SelectionGpPlace;
extern s16 ReverseCourse;
extern s16 CourseIndex;
extern RrTimeRecord16 RaceRecords[2][8][5];
extern RrTimeRecord16 LapRecords[2][8][5];
extern u8 *CarRecordPtrs[];
extern s32 TotalRaceTime;


s32 InitMenuTextRows(RrMenuTextRow60 *rows) {
    u32 size = (u32)MenuTextRowCount * 60U;
    MenuTextRows = rows;
    PsyqBzero(rows, size);
    return (s32)size;
}

void SetMenuTextRow(s32 row, s32 position, s32 phase, s32 style, s32 label) {
    s32 templateIndex;
    s32 formatKind;
    s32 column;
    s32 value;
    s32 suffixIndex;

    if (row < MenuTextRowCount) {
        templateIndex = label & 0xFF;
        formatKind = label >> 8;
        MenuTextRows[row].rendering.phase = phase;
        column = 0;
        MenuTextRows[row].rendering.style = style;
        MenuTextRows[row].rendering.x = 0x11;
        MenuTextRows[row].rendering.y = (s16)position * 28 + 0x60;
        MenuTextRows[row].rendering.clut = 0x2638;
        if (label >= 0) {
            while (MenuTextTemplates[templateIndex][column] != 0) {
                MenuTextRows[row].rendering.text[column] = MenuTextTemplates[templateIndex][column];
                column++;
            }
        } else {
            for (; column < 0x16; column++) {
                MenuTextRows[row].rendering.text[column] = 0;
            }
            return;
        }
        switch (formatKind) {
        case 1:
        case 2:
        case 3:
            FormatDecimalDigits((char *)&MenuTextRows[row].rendering.text[4], formatKind, 0);
        case 4:
            FormatRaceTime((char *)&MenuTextRows[row].rendering.text[7], TotalRaceTime);
            break;
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
            value = (formatKind - 5) % 5;
            FormatDecimalDigits((char *)&MenuTextRows[row].rendering.text[0], value + 1, 0);
            suffixIndex = value;
            if (value >= 3) {
                suffixIndex = 3;
            }
            CopyStringClearTail((char *)&MenuTextRows[row].rendering.text[1], OrdinalSuffixStrs[suffixIndex]);
            CopyStringClearTail((char *)&MenuTextRows[row].rendering.text[4], LapTimeTemplateStr);
            if (formatKind < 10) {
                CopyStringClearTail((char *)&MenuTextRows[row].rendering.text[14], LapRecords[ReverseCourse][CourseIndex][value].name);
                FormatRaceTime((char *)&MenuTextRows[row].rendering.text[4], LapRecords[ReverseCourse][CourseIndex][value].time);
            } else {
                CopyStringClearTail((char *)&MenuTextRows[row].rendering.text[14], RaceRecords[ReverseCourse][CourseIndex][value].name);
                FormatRaceTime((char *)&MenuTextRows[row].rendering.text[4], RaceRecords[ReverseCourse][CourseIndex][value].time);
            }
            break;
        case 15:
            value = CarRecordPtrs[0][0x1F3];
            FormatDecimalDigits((char *)&MenuTextRows[row].rendering.text[12], value + 1, 0);
            break;
        case 16:
            FormatDecimalDigits((char *)&MenuTextRows[row].rendering.text[8], SelectionState.retries08[SelectionState.phase17], 0);
            break;
        case 17:
            value = SelectionGpPlace;
            CopyStringClearTail((char *)&MenuTextRows[row].rendering.text[0], StrStarSpace);
            if (value == 0) {
                value = 1;
            }
            if (value < 9) {
                FormatDecimalDigits((char *)&MenuTextRows[row].rendering.text[1], value, 0);
                value--;
                if (value >= 3) {
                    value = 3;
                }
                CopyStringClearTail((char *)&MenuTextRows[row].rendering.text[2], OrdinalSuffixStrs[value]);
            } else {
                CopyStringClearTail((char *)&MenuTextRows[row].rendering.text[1], StrRetirePtr);
            }
            break;
        case 18:
        case 19:
        case 20:
        case 21:
            value = formatKind - 18;
            FormatDecimalDigits((char *)&MenuTextRows[row].rendering.text[1], value + 1, 0);
            if (value >= 3) {
                value = 3;
            }
            CopyStringClearTail((char *)&MenuTextRows[row].rendering.text[2], OrdinalSuffixStrs[value]);
            break;
        case 30:
            MenuTextRows[row].rendering.clut = 0x2639;
            MenuTextRows[row].rendering.flags = 3;
            break;
        case 31:
            MenuTextRows[row].rendering.clut = 0x2639;
            MenuTextRows[row].rendering.flags = 2;
            MenuTextRows[row].rendering.text[0] = MenuTextTemplates[templateIndex][0];
            MenuTextRows[row].rendering.text[1] = 0;
            break;
        }
    }
}
