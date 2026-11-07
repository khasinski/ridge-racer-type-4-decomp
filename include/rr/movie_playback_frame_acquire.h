#ifndef RR_MOVIE_PLAYBACK_FRAME_ACQUIRE_H
#define RR_MOVIE_PLAYBACK_FRAME_ACQUIRE_H
#include "rr/movie_playback_stream.h"
#include "rr/frame_storage_types.h"
extern RrFrameStorage FrameEnvs[2];
void ClearImage(const RrGpuRectangleFields *,u8,u8,u8);
/* Poll at most2000 times. A successful header has the full queue-record
 * owner; activeB8 points to a complete playback state. Config and both frame
 * owners stay live across clear-image callbacks, which may replace activeB8.
 * Width arithmetic wraps32 bits before signed division and narrowing. */
u8 *movie_playback_next_frame(RrStrDisplayConfig *config);
#endif
