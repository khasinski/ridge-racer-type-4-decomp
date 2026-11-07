#include "rr/movie_playback_track_project.h"
/* The initializer supplies its position pointer; this body uses globals. */
void movie_playback_camera_locate_track(s32 *unused_position) {
    RrTrackOffsetArgument offsets[2];
    s32 position[3];
    s32 height;
    s32 idx;

    (void)unused_position;

    position[0] = RaceContext.position450[0] >> 2;
    position[1] = RaceContext.position450[1] >> 2;
    position[2] = RaceContext.position450[2] >> 2;
    idx = FindTrackNode(position, RaceContext.node498, (s32)((u32)TrackNodeCount + 1U));
    RaceContext.node498 = idx;
    offsets[0].value = (RaceContext.position450[0] / 4) - (u16)TrackNodes[idx].x08;
    offsets[1].value = (RaceContext.position450[2] / 4) - (u16)TrackNodes[idx].z0C;
    ProjectToTrackSegment(offsets[0], offsets[1], RaceContext.node498, &RaceContext.fraction49C, &RaceContext.lateral49E, &height);
}
