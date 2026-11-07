#ifndef RR_OVERLAY660_H
#define RR_OVERLAY660_H

#include "rr/menu_overlay_runtime.h"
#include "rr/overlay_race_state.h"
#include "rr/race_frame_scratch.h"

extern const char time_attack_course_select_title[];
extern char time_attack_course_select_new_record_label[];
extern s32 time_attack_course_select_result_labels[55];
extern u8 time_attack_course_select_preview_row_map[2][4];
extern s16 time_attack_course_select_available_car_counts[2];
extern s32 time_attack_course_select_available_course_count;
extern s32 time_attack_course_select_race_record_rank;
extern s32 time_attack_course_select_name_cursor;
extern s16 time_attack_course_select_result_kind;
extern char time_attack_course_select_record_name[12];
extern s16 time_attack_course_select_preview_confirmation;
extern s32 time_attack_course_select_result_transition;
extern s16 time_attack_course_select_preview_selection;
extern s32 time_attack_course_select_lap_record_rank;
extern s16 time_attack_course_select_preview_bank;

extern s32 GameLanguage;
extern s16 SceneId;
extern s32 MenuCancelPressed, Pad0Protocol;
extern s16 MenuReturnModeCode, MenuResultCode;
void CopyStringClearTail(char *destination, char *source);
void StepNameChar(char *text);
void DrawSplitTimeHud(s32 paused);
s32 CountChoiceGlyphs(const RrCourseChoice4 *choices);
s32 EncodeRecordPassword(s32 course, s32 selection, s32 car, s32 mode, s32 value, u8 *output);

void time_attack_course_select_set_scene_id(void);
void time_attack_course_select_init_menu(void);
void time_attack_course_select_init_scene(void);
void time_attack_course_select_update_menu(void);
void time_attack_course_select_init_results(void);
/* The results controller explicitly passes zero in a0. */
s32 time_attack_course_select_update_results_choice(s32 reserved);
void time_attack_course_select_update_results(void);
s32 time_attack_course_select_insert_record(s32 mode, s32 time);
void time_attack_course_select_find_best_lap(void);
void time_attack_course_select_init_preview(void);
void time_attack_course_select_update_preview(void);
void time_attack_course_select_exit_race(void);
void time_attack_course_select_update_race(void);

typedef RrRaceFrameScratch Rr660FrameScratch;
#ifndef RR_660_FRAME_SCRATCH
#define RR_660_FRAME_SCRATCH ((RrRaceFrameScratch *)0x1F800000U)
#endif

#endif
