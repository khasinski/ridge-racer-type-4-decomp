#ifndef RR_OVERLAY660_H
#define RR_OVERLAY660_H

#include "rr/menu_overlay_runtime.h"
#include "rr/overlay_race_state.h"
#include "rr/race_frame_scratch.h"

extern const char ovl660_title[];
extern char ovl660_new_record_label[];
extern s32 ovl660_result_labels[55];
extern u8 ovl660_preview_row_map[2][4];
extern s16 ovl660_available_car_counts[2];
extern s32 ovl660_available_course_count;
extern s32 ovl660_race_record_rank;
extern s32 ovl660_name_cursor;
extern s16 ovl660_result_kind;
extern char ovl660_record_name[12];
extern s16 ovl660_preview_confirmation;
extern s32 ovl660_result_transition;
extern s16 ovl660_preview_selection;
extern s32 ovl660_lap_record_rank;
extern s16 ovl660_preview_bank;

extern s32 GameLanguage;
extern s16 SceneId;
extern s32 MenuCancelPressed, D_800F3A48;
extern s16 MenuReturnModeCode, MenuResultCode;
void CopyStringClearTail(char *destination, char *source);
void StepNameChar(char *text);
void DrawSplitTimeHud(s32 paused);
s32 CountChoiceGlyphs(const RrCourseChoice4 *choices);
s32 EncodeRecordPassword(s32 course, s32 selection, s32 car, s32 mode, s32 value, u8 *output);

void ovl660_func_80114910(void);
void ovl660_func_80114920(void);
void ovl660_func_80114C40(void);
void ovl660_func_80114D38(void);
void ovl660_func_801154D8(void);
/* The results controller explicitly passes zero in a0. */
s32 ovl660_func_801156A8(s32 reserved);
void ovl660_func_8011572C(void);
s32 ovl660_func_80115FA8(s32 mode, s32 time);
void ovl660_func_801162D8(void);
void ovl660_func_801163C4(void);
void ovl660_func_80116530(void);
void ovl660_func_8011712C(void);
void ovl660_func_8011718C(void);

typedef RrRaceFrameScratch Rr660FrameScratch;
#ifndef RR_660_FRAME_SCRATCH
#define RR_660_FRAME_SCRATCH ((RrRaceFrameScratch *)0x1F800000U)
#endif

#endif
