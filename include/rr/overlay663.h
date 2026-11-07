#ifndef RR_OVERLAY_663_H
#define RR_OVERLAY_663_H

#include "rr/menu_overlay_runtime.h"

extern const char records_scene_name[];
extern u8 records_course_order[9];
extern u8 records_time_text[10];
extern RrUiUvRect records_record_badges[2];
extern u8 records_race_column_widths[4];
extern u8 records_lap_column_widths[4];
extern RrUiUvRect records_record_label;
extern s32 records_selected_record;
extern s32 records_course_count;
extern s32 records_available_courses[9];
extern s32 records_extra_courses_enabled;
extern s32 records_record_kind;
extern s32 records_available_course_count;
extern s32 records_course_unlock_levels[9];

void records_init_menu(void);
void records_update_menu(void);
void records_init_selection(s32);
void records_update_selection(void);
void records_init_preview(void);
void records_draw_preview(void);
void records_update_preview(void);
void records_init_result_preview(void);
void records_update_result_preview(void);
u8 *records_format_time(s32 milliseconds);
void *records_draw_race_records(u32 *, void *, s32);
void *records_draw_lap_records(u32 *, void *, s32);
void *records_draw_record_labels(u32 *, void *, s32);
void records_init_scene(void);

#endif
