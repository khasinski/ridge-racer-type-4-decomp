#ifndef RR_OVERLAY661_COURSE_MENU_H
#define RR_OVERLAY661_COURSE_MENU_H
#include "rr/ui.h"
#include "rr/icon_sprites.h"
#include "rr/tables.h"
#include "rr/selection_types.h"
#include "rr/menu_selection_word.h"
#include "rr/menu_arena_types.h"
extern s32 ScreenFrame,ModeFrame,NumLaps,D_800F4CB8,ovl661_D_80118414;
extern s16 CourseMenuSelection,MenuCursor,MenuItemCount,MenuFadeTimer,CourseIndex,ModeId,MenuScriptId,ModeGroup,RaceType;
extern s16 ovl661_D_80118448[2];
extern s16 SelectedCarIndex;
extern u16 D_800F4D8A;
extern RrMenuSelectionWord32 NumCars;
extern RrSelectionRows SelectionRows;
extern RrMenuItemState5C PlayerControllers[2];
extern RrMenuArenas MenuArenas;
extern void *LoadCursor;
extern u8 D_80118374[4];
void SetModelTexturePage(s32);void AddScreenFadeTiles(u32*,s32,u32);void UpdateScreenFade(s32);void SetMenuFrameOptions(s32,s32,s32);
void DrawMenuStepArrows(s32,s32,s32,s32,s32,s32,s32);void StepMenuValue(s32,s32,char*,s32,s32);
s32 DecodeCourseIndex(s32);void DrawCourseInfoPanel(s32,s32);void DrawCourseMapPreview(s32,s32);void ResetScreenCounters(void);s32 LoadCarRangeStep(s32,s32);s32 LoadTrackNodesStep(void);void DrawUiRow(s32);void PlaySfx(s32);void SeqFadeVolume(s32,s32);void SeqMuteVolume(void);void ovl661_func_80115598(s32);
/* UI roots cover at least26 complete rows. Highlight count is0..10; its byte span may extend beyond cells[8]
 * into later bytes of the same complete row.
 * Parameter table indices are0..3. Menu phase0 accepts selection0..3; phase5
 * with row24 state2 requires selection0..1 before each indexed access,
 * including after services. Confirmation sets selection2 together with row
 * state3, leaving that indexed branch. AC59C owns a complete word edited by
 * width4 input; signed low-half reads use its union member. Input .a owns the
 * resident aliasF3A60. Services may replace roots and valid live selectors. */
void ovl661_func_80117430(void);
RR_ASSERT_SIZE(RrSelectionRows,32);
#endif
