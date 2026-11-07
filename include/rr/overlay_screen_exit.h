#ifndef RR_OVERLAY_SCREEN_EXIT_H
#define RR_OVERLAY_SCREEN_EXIT_H
#include "rr/frame_environment.h"
#include "rr/screen_tiles.h"

extern u16 ModeGroup,ModeId;
extern s16 ModeSubState;
void StopRaceAudioAndRumble(void),SetupRaceAndScene(void);
void serial_link_exit_race(void);
#endif
