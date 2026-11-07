#include "rr/overlay663.h"

u8 *records_format_time(s32 milliseconds) {
    s32 seconds = milliseconds / 1000;
    s32 minutes = seconds / 60;
    s32 remainder = seconds % 60;
    seconds = milliseconds - seconds * 1000;
    records_time_text[0] = minutes / 10 + '0';
    records_time_text[1] = minutes % 10 + '0';
    records_time_text[3] = remainder / 10 + '0';
    records_time_text[4] = remainder % 10 + '0';
    records_time_text[6] = seconds / 100 + '0';
    records_time_text[7] = seconds / 10 % 10 + '0';
    records_time_text[8] = seconds % 10 + '0';
    return records_time_text;
}

void *records_draw_race_records(u32 *ordering, void *cursor, s32 row) {
    s32 i = 0, x, y, clut, rowY;
    s32 group = CourseMenuSelection >= 8;
    y = BadgeRows[row].y06 + 40;
    do {
        if (i == records_selected_record) {
            if (rcos((s32)(((u32)ScreenFrame << 7) - 682U)) < 0)
                clut = 32761;
            else
                clut = 32760;
        } else
            clut = 32760;
        x = BadgeRows[row].x04 + 8;
        cursor = AddSpritePrim(ordering, cursor, x, y, &D_8009FAA0[i], clut, 0);
        x += records_race_column_widths[0];
        cursor = AddTilePrim(ordering, cursor, x, y - 2, 1, 20, 255, 255, 255);
        x += records_race_column_widths[1];
        cursor = AddTextSprites(ordering, cursor, x, y,
                               (char *)records_format_time(RaceRecords[group][CourseIndex][i].time),
                               clut, 0, 0);
        x += records_race_column_widths[2];
        cursor = AddTilePrim(ordering, cursor, x, y - 2, 1, 20, 255, 255, 255);
        x += records_race_column_widths[3];
        cursor = AddTextSprites(ordering, cursor, x, y, RaceRecords[group][CourseIndex][i].name, clut,
                               0, 0);
        y += 24;
        i++;
    } while (i < 5);
    x += 52;
    rowY = BadgeRows[row].y06;
    y = rowY + 40;
    cursor = AddTilePrim(ordering, cursor, x, y - 2, 1, 116, 255, 255, 255);
    cursor = AddDrawModePrim(ordering, cursor, 56);
    x += 16;
    cursor = DrawCourseIconSprite(ordering, cursor, x, y, CourseMenuSelection & 7);
    cursor = AddSpritePrim(ordering, cursor, x + 8, rowY + 112, &records_record_badges[group],
                           group + 30180, 0);
    return AddDrawModePrim(ordering, cursor, 89);
}

void *records_draw_lap_records(u32 *ordering, void *cursor, s32 row) {
    s32 i = 0, x, y, clut, rowY;
    s32 group = CourseMenuSelection >= 8;
    y = BadgeRows[row].y06 + 40;
    do {
        if (i == records_selected_record) {
            if (rcos((s32)(((u32)ScreenFrame << 7) - 682U)) < 0)
                clut = 32761;
            else
                clut = 32760;
        } else
            clut = 32760;
        x = BadgeRows[row].x04 + 8;
        cursor = AddSpritePrim(ordering, cursor, x, y, &D_8009FAA0[i], clut, 0);
        x += records_lap_column_widths[0];
        cursor = AddTilePrim(ordering, cursor, x, y - 2, 1, 20, 255, 255, 255);
        x += records_lap_column_widths[1];
        cursor = AddTextSprites(ordering, cursor, x, y,
                               (char *)records_format_time(LapRecords[group][CourseIndex][i].time),
                               clut, 0, 0);
        x += records_lap_column_widths[2];
        cursor = AddTilePrim(ordering, cursor, x, y - 2, 1, 20, 255, 255, 255);
        x += records_lap_column_widths[3];
        cursor = AddTextSprites(ordering, cursor, x, y, LapRecords[group][CourseIndex][i].name, clut,
                               0, 0);
        y += 24;
        i++;
    } while (i < 5);
    x += 52;
    rowY = BadgeRows[row].y06;
    y = rowY + 40;
    cursor = AddTilePrim(ordering, cursor, x, y - 2, 1, 116, 255, 255, 255);
    cursor = AddDrawModePrim(ordering, cursor, 56);
    x += 16;
    cursor = DrawCourseIconSprite(ordering, cursor, x, y, CourseMenuSelection & 7);
    cursor = AddSpritePrim(ordering, cursor, x + 8, rowY + 112, &records_record_badges[group],
                           group + 30180, 0);
    return AddDrawModePrim(ordering, cursor, 89);
}

void *records_draw_record_labels(u32 *arg0, void *arg1, s32 arg2) {
    char buf[8];
    s32 i;
    s32 x;
    s32 y;

    x = BadgeRows[arg2].x04 + 8;
    y = BadgeRows[arg2].y06 + 32;
    for (i = 0; i < 8; i++) {
        buf[i] = 0;
    }
    arg1 = AddSpritePrim(arg0, arg1, x, y, &records_record_label, 0x7FF8, 0);
    x += records_record_label.w;
    FormatDecimalDigits(buf, records_course_unlock_levels[records_selected_record], 1);
    arg1 = AddTextSprites(arg0, arg1, x, y, buf, 0x7FF8, 0, 0);
    return AddDrawModePrim(arg0, arg1, 56);
}
