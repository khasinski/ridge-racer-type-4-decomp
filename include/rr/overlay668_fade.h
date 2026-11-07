#ifndef RR_OVERLAY668_FADE_H
#define RR_OVERLAY668_FADE_H
#include "types.h"
/* Signed16 frame:0 before48, ramp to256 at64, plateau through140,
 * ramp down to0 at156, then remain0. Result is a multiple of16 in0..256. */
s32 movie_playback_credits_row_fade(s16 frame);
#endif
