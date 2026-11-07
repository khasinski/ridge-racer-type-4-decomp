#ifndef RR_OVERLAY667_SELECTION_INIT_H
#define RR_OVERLAY667_SELECTION_INIT_H
#include "rr/ui.h"
#include "rr/race_types.h"
#include "rr/menu_arena_types.h"
#include "rr/menu_selection.h"
#include "rr/selection_types.h"
#include "rr/course_choice_types.h"
#include "rr/projection_state.h"
extern s32 ModeFrame,ScreenFrame;
extern s16 MenuCursor,MenuItemCount,D_8010C0B8;
extern u8 *CdDirectoryBuffer;
extern RrMenuArenas MenuArenas;
extern void *LoadCursor,*MenuArenaBase;
extern RrRaceVehicle PlayerCars[2];
extern RrSelectionRows SelectionRows;
extern RrCourseChoice4 D_8010BFD8[2][2][9];
extern s16 D_8010C0C0[2][2],D_8010C0D8[2],CourseMenuCursor[2];
void ResetScreenCounters(void);
void InitMenuArenas(void);
void SetupRenderer(s32);
void InitPreviewImageRows(void);
void ResetCarState(RrRaceVehicle *);
s32 CountChoiceGlyphs(const RrCourseChoice4 *);
void UnpackChoiceGlyph(s8 *,s32);
#ifndef RR_PROJECTION_SCRATCH
#define RR_PROJECTION_SCRATCH ((RrProjectionScratchPrefix *)0x1F800000)
#endif
/* Selection initializer: original player0..3 selects the group reset. Service
 * callbacks may change the live player0..3, arena roots and choice state;
 * every subsequent publication uses that live state. Group/index must stay
 * within2 groups and9 choices. Resource base owns at least0x2D000 bytes.
 * Pointer fields retain native widths; only the target has PAL pointer layout. */
RR_ASSERT_SIZE(RrRaceVehicle,0x320);
RR_ASSERT_SIZE(RrCourseChoice4,4);
RR_ASSERT_SIZE(RrSelectionRows,32);
void ovl667_func_801161A8(s32 player);
#endif
