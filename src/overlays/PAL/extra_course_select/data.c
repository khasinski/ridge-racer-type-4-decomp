#include "rr/overlay666.h"

u32 extra_course_select_module_id __attribute__((section(".extra_course_select.header"))) = 15;

const char extra_course_select_scene_name[24] __attribute__((section(".extra_course_select.scene_name"))) =
    "job_ex_crs_info0\n";

RrUiUvRect extra_course_select_selection_sprite
    __attribute__((section(".extra_course_select.selection_sprite"))) = {0, 120, 40, 80};

s32 extra_course_select_result_labels[33] __attribute__((section(".extra_course_select.result_labels"))) = {
    7702, 0,  -1, 3,  5,    23,   -1, -1,   -1, 7702, -1, 7702, 0,  -1,   19, -1, 28,
    29,   30, 31, -1, 7702, 7702, 17, 7702, -1, 6,    8,  9,    -1, 7702, -1, -1};

u16 extra_course_select_course_dispatch[4][4] __attribute__((section(".extra_course_select.course_dispatch"))) = {
    {6, 1, 6, 6}, {0, 0, 0, 3}, {4, 4, 7, 4}, {5, 2, 2, 2}};

s16 extra_course_select_course_counts[4] __attribute__((section(".extra_course_select.course_counts"))) = {0, 0, 0, 0};

s32 extra_course_select_preview_phase __attribute__((section(".extra_course_select.preview_phase"))) = 0;

s32 extra_course_select_result_kind __attribute__((section(".extra_course_select.result_kind"))) = 0;

RrMenuSelectionWord32 extra_course_select_selected_course
    __attribute__((section(".extra_course_select.selected_course"))) = {0};

RrCourseChoice4 extra_course_select_available_courses[4][9]
    __attribute__((section(".extra_course_select.available_courses"))) = {0};
