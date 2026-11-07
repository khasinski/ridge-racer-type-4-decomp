#ifndef RR_OVERLAY661_LAYOUT_H
#define RR_OVERLAY661_LAYOUT_H
#include "rr/race.h"
#include "rr/projection_state.h"
#include "rr/renderer_setup.h"
#include "rr/menu_arena_types.h"
#include "rr/menu_selection.h"
#include "rr/selection.h"
#include "rr/course_choice_types.h"
#include "rr/menu_glyph.h"
#include "rr/ui.h"
extern RrMenuArenas MenuArenas;
extern u8 *CdDirectoryBuffer;
extern void *MenuArenaBase,*LoadCursor;
extern s32 ModeFrame,ScreenFrame;
extern s16 MenuCursor,MenuItemCount,versus_select_D_80118438;
extern s16 versus_select_choice_counts[2][2],versus_select_choice_index[2],versus_select_choice_group[2];
extern RrCourseChoice4 versus_select_choice_tables[2][2][9];
void ResetScreenCounters(void);
void InitMenuArenas(void);
void InitPreviewImageRows(void);
void ResetCarState(RrRaceVehicle *);
s32 CountChoiceGlyphs(const RrCourseChoice4 *);
/* Mode and fresh player selectors are0..1. Each group is0..1 and course0..8;
 * count helpers inspect all nine records. The full selection owner has32 bytes.
 * Setup callbacks may replace arenas and mutate every later selection. */
void versus_select_init_selection_screen(s32 mode);
#ifndef RR_PROJECTION_SCRATCH
#define RR_PROJECTION_SCRATCH ((RrProjectionScratchPrefix *)0x1F800000U)
#endif
#endif
