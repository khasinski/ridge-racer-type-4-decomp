#ifndef RR_MENU_SCREEN_EXIT_H
#define RR_MENU_SCREEN_EXIT_H
#include "rr/frame_environment.h"
#include "rr/screen_tiles.h"
#include "game/race/raceset.h"
/* These scene exits draw through the containing frame's ordering table.
 * The first rectangle is the published screen context. */
extern u16 ModeGroup;
extern s16 ModeId,ModeSubState;
void StopRaceAudioAndRumble(void);
void time_attack_course_select_exit_race(void);
void versus_select_func_80114AA0(void);
#endif
