#ifndef RR_MOVIE_PLAYBACK_PLAYBACK_STEP_H
#define RR_MOVIE_PLAYBACK_PLAYBACK_STEP_H
#include "rr/movie_playback_stream.h"
#include "rr/diagnostics.h"
#include "rr/tables.h"
extern RrMenuItemState5C PlayerControllers[];
extern const char movie_playback_str_next_timeout_fmt[];
s32 StGetBackloc(const RrCdLocation *location);
void DrawSync(s32 mode);
/* activeB8 addresses a complete playback owner, and command/display selectors
 * word00/word08 are0..1 at each use. Owners and currentC0 remain live across
 * services, which may replace activeB8 and change thresholds, state or input.
 * Retry until the next-frame service succeeds or unsigned threshold is reached.
 * Timer and frame words retain 32-bit wrapping and signed/unsigned comparisons. */
void movie_playback_step(void);
#endif
