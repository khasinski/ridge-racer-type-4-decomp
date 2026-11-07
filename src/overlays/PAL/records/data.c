#include "rr/records.h"

u32 records_module_id __attribute__((section(".records.header"))) = 13;

const char records_scene_name[16] __attribute__((section(".records.scene_name"))) = "recode0\n";

u8 records_course_order[9]
    __attribute__((section(".records.course_order"))) = {1, 0, 2, 3, 8, 4, 6, 7, 5};

u8 records_time_text[10]
    __attribute__((section(".records.time_text"))) = {48, 48, 39, 48, 48, 34, 48, 48, 48, 0};

RrUiUvRect records_record_badges[2]
    __attribute__((section(".records.record_badges"))) = {{0, 216, 16, 32}, {16, 216, 16, 32}};

u8 records_race_column_widths[4]
    __attribute__((section(".records.race_column_widths"))) = {20, 8, 60, 8};

u8 records_lap_column_widths[4]
    __attribute__((section(".records.lap_column_widths"))) = {20, 8, 60, 8};

RrUiUvRect records_record_label __attribute__((section(".records.record_label"))) = {176, 16, 48, 16};

s32 records_selected_record __attribute__((section(".records.selected_record"))) = 0;

s32 records_course_count __attribute__((section(".records.course_count"))) = 0;

s32 records_available_courses[9]
    __attribute__((section(".records.available_courses"))) = {0, 0, 0, 0, 0, 0, 0, 0, 0};

s32 records_extra_courses_enabled __attribute__((section(".records.extra_courses_enabled"))) = 0;

s32 records_record_kind __attribute__((section(".records.record_kind"))) = 0;

s32 records_available_course_count __attribute__((section(".records.available_course_count"))) = 0;

s32 records_course_unlock_levels[9]
    __attribute__((section(".records.course_unlock_levels"))) = {0, 0, 0, 0, 0, 0, 0, 0, 0};
