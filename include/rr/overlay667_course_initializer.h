#ifndef RR_OVERLAY667_COURSE_INITIALIZER_H
#define RR_OVERLAY667_COURSE_INITIALIZER_H
#include "rr/layout.h"
#include "rr/course_choice_types.h"
#include "rr/menu_choice_types.h"
#include "rr/selection.h"
#include "rr/player_status.h"
#include "rr/course_choices.h"
extern s16 PlayerWinCounts[4],CourseMenuCursor[2],CourseMenuPage[2];
extern RrSelectionRows SelectionRows;
extern RrCourseChoice4 CourseMenuChoices[2][2][9];
extern RrCourseChoice4 CourseChoiceTable[2][9];
extern char PlayerNames[4][12];
extern char *serial_link_default_player_names[4];
extern s8 LinkEntryState[2][2];
extern s32 NumLaps,SelectionPlayer,AvailableCourseCount;
extern s16 CourseIndex,CourseMenuSelection,ModeId,CourseMenuPhase,LinkCourseMenuEntryMode;
extern u16 ModeGroup,ModeSubState;
void SetDispMask(s32);void SetupDrawDispEnvs(s32,s32,s16,u32);s32 UpdateLinkHandshake(s32);void ClearScreenOverlayState(void);
void CopyStringClearTail(char*,char*);void serial_link_init_car_select(s32);void UnpackChoiceGlyph(s8*,s32);void SeqStartTrack(s32);void SetMenuFrameOptions(s32,s32,s32);
/* Four complete selection prefixes occupy bytes 0..15 of the 32-byte owner.
 * Each player base freshly read after a service is 0 or 2; mode is 0 or 1.
 * Name lookups are four native pointers to strings fitting twelve bytes.
 * Copies cover the full 144-byte two-snapshot/two-player/nine-course owner. */
void serial_link_init_scene(void);
#endif
