#ifndef RR_MOVIE_PLAYBACK_TRACK_PROJECT_H
#define RR_MOVIE_PLAYBACK_TRACK_PROJECT_H
#include "rr/movie_playback_camera.h"
#include "rr/track_projection.h"
s32 FindTrackNode(s32 *position,s32 node,s32 count);
/* Search uses arithmetic-shifted coordinates; interpolation uses division
 * toward zero. Reload positions and the sample root after the search service.
 * The returned node addresses a complete track sample. Coordinate arguments
 * retain their low sixteen bits; all software pointers remain full width. */
#endif
