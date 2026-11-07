#ifndef RR_OVERLAY668_DISPATCH_H
#define RR_OVERLAY668_DISPATCH_H
#include "rr/menu_choice.h"
#include "rr/overlay668_decode.h"
/* Current playback phase:0 starts from the published resource buffer,
 *1 updates playback,2 shuts it down. Other signed word values are idle. */
extern s32 RaceContextMoviePhase;
void movie_playback_update(void);
void movie_playback_start(u8 *resource);
void movie_playback_step(void);
#endif
