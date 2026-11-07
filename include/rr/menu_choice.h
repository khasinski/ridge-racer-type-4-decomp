#ifndef RR_MENU_CHOICE_H
#define RR_MENU_CHOICE_H

#include "rr/ui.h"
#include "rr/badge_rows.h"

/* Byte-aligned records deliberately use the original unaligned word transfers. */
#include "rr/menu_state_types.h"
#include "game/menu/menuwork.h"

extern s32 MenuStepDirection, MenuConfirmPressed;
extern void *LoadCursor, *MenuArenaBase;
extern RrMenuSettings2C SelectionState;
extern s16 CourseListIndex;
extern u16 Pad0RepeatButtons;
extern RrMenuGlyphRow1C *GlyphRows;
extern RrMenuChoice4 SelectionRows;
extern u8 CourseListSavedIndex; /* Alias of SelectionRows.bytes[1]. */
extern u8 *Dialogue;
void PlaySfx(s32 mode);
void ResetScreenCounters(void);
void SetMenuFrameOptions(s32 first, s32 second, s32 third);

/* Current UI backing must cover byte12+the initial byte selection (up to267).
 * Movement requires four glyph rows and byte4 of the highlight buffer. Helpers
 * may change selection, backing pointers and records. Acceptance uses captured
 * state3 but fresh row/UI pointers after the sound helper. Publishes the fresh
 * full buffer pointer after both state helpers. */
void UpdateCourseListChoice(void);
RR_ASSERT_SIZE(RrMenuChoice4, 4);
RR_ASSERT_SIZE(RrMenuSettings2C, 0x2C);
RR_ASSERT_OFFSET(RrMenuSettings2C, retries08, 8);
RR_ASSERT_OFFSET(RrMenuSettings2C, phase17, 0x17);
RR_ASSERT_OFFSET(RrMenuSettings2C, mode1A, 0x1A);
RR_ASSERT_OFFSET(RrMenuSettings2C, copies, 0x1C);
RR_ASSERT_SIZE(RrMenuGlyphRow1C, 0x1C);
RR_ASSERT_OFFSET(RrMenuGlyphRow1C, control02, 2);
RR_ASSERT_OFFSET(RrMenuGlyphRow1C, glyph07, 7);
RR_ASSERT_OFFSET(RrMenuGlyphRow1C, x08, 8);
RR_ASSERT_OFFSET(RrMenuGlyphRow1C, phase18, 0x18);

#endif
