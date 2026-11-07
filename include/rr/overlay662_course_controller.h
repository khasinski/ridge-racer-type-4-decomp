#ifndef RR_OVERLAY662_COURSE_CONTROLLER_H
#define RR_OVERLAY662_COURSE_CONTROLLER_H
#include "rr/race.h"
#include "rr/menu_transition.h"
#include "rr/menu_selection.h"
#include "rr/ui.h"
#include "rr/badge_rows.h"
#include "rr/tables.h"
#include "rr/selection_types.h"
#include "rr/course_choice_types.h"
#include "rr/menu_arena_types.h"
#include "rr/preview_rgb.h"
#include "rr/frame_storage_types.h"
extern RrCourseChoice4 CourseChoiceTable[2][9];
extern RrMenuItemState5C PlayerControllers[2];
extern RrSelectionRows SelectionRows;
extern RrMenuArenas MenuArenas;
extern void *LoadCursor;
extern u8 *ImageRows;
extern s32 ModeFrame,SelectionPlayer;
extern s16 MenuCursor,ModeId;
extern u16 MenuFadeTimer,ModeGroup;
extern s16 garage_D_801272D4,garage_car_list_count,garage_car_index;
extern u16 garage_replace_slot_index;
void SetupMenuCarCamera(s32);void SetupViewport(s32);s32 LoadCarRangeStep(s32,s32);
void SetCarBounds(RrRaceVehicle*);void PrepareCarDrawPose(char*);
void SetMenuFrameOptions(s32,s32,s32);void StepMenuValue(s32,s32,char*,s32,s32);
void UnpackChoiceGlyph(s8*,s32);s32 PackChoiceGlyph(void*);
void ResetScreenCounters(void);void SetModelTexturePage(s32);void PlaySfx(s32);
void DrawUiRow(s32);void DrawSelectionIconGrid(s32);
void DrawPreviewCar(RrRaceVehicle*,RrPreviewRgb3,s32,s32);
void UpdateBadgeRowAnim(s32);s32 UpdateImageRow(s32);
void DrawMenuStepArrows(s32,s32,s32,s32,s32,s32,s32);void UpdateScreenFade(s32);
void AddScreenFadeTiles(u32*,s32,u32);void garage_enter_car_select(void);
/* UI selection is0..3 and available-car selection0..8. Dynamic close counts
 * require initialized rows. Services may replace full UI, badge, frame, mode
 * and arena roots; the complete mode allocation and32-byte selection owner
 * remain live. Readiness is a full signed word; RGB has three defined bytes. */
void garage_update_car_info(void);
#endif
