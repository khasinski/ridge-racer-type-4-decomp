#ifndef RR_OVERLAY_666_H
#define RR_OVERLAY_666_H

#include "rr/menu_overlay_runtime.h"

extern const char extra_course_select_scene_name[];
extern RrUiUvRect extra_course_select_selection_sprite;
extern s32 extra_course_select_result_labels[33];
extern u16 extra_course_select_course_dispatch[4][4];
extern s16 extra_course_select_course_counts[4];
extern s32 extra_course_select_preview_phase;
extern s32 extra_course_select_result_kind;
extern RrMenuSelectionWord32 extra_course_select_selected_course;
extern RrCourseChoice4 extra_course_select_available_courses[4][9];

void extra_course_select_init_menu(void);
void extra_course_select_update_menu(void);
void extra_course_select_update_course_selection(void);
void extra_course_select_draw_selection_pulse(s32, s32);
s32 extra_course_select_update_retry_choice(s32);
void extra_course_select_update_results(void);
void extra_course_select_init_preview(void);
void extra_course_select_update_preview(void);
void extra_course_select_update_checkpoint(RrRaceCheckpointVehicle *vehicle);
void extra_course_select_update_race(void);
void extra_course_select_init_scene(void);
void extra_course_select_init_results(void);
void extra_course_select_exit_race(void);

#endif
