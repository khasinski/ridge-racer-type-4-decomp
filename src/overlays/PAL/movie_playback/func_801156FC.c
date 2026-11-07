#include "rr/overlay668_camera.h"

void movie_playback_camera_update_pitch(void) {
    s32 sp18[4];
    s32 sp28[4];
    s32 sp38[4];
    s32 unused[2];
    s32 idx;
    /* ASM_BARRIER: Keep the neighbour index in v1 across the two sample calculations. */
    register s32 next __asm__("$3");
    s32 dx;
    s32 dz;

    idx = RaceContext.node498;
    next = idx + 1;
    if (next >= TrackNodeCount) {
        next = 0;
    }
    InterpolateLanePointXYZ(&TrackNodes[idx], &TrackNodes[next], RaceContext.lateral49E, RaceContext.fraction49C, sp18);
    next = RaceContext.node498;
    idx = next - 1;
    RaceContext.position450[1] = (s32)((u32)sp18[1] - 120U);
    if (idx < 0) {
        idx += TrackNodeCount;
    }
    InterpolateLanePointXYZ(&TrackNodes[idx], &TrackNodes[next], RaceContext.lateral49E, RaceContext.fraction49C, sp28);
    dx = (s32)(((u32)sp28[0] - (u32)sp18[0]) << 2);
    sp38[0] = dx;
    dz = (s32)(((u32)sp28[2] - (u32)sp18[2]) << 2);
    sp38[2] = dz;
    sp38[1] = (s32)((u32)sp28[1] - (u32)sp18[1]);
    RaceContext.pitch460 = 1024 - Atan2Table(sp38[1], CordicSqrt((s32)((u32)dx * (u32)dx + (u32)dz * (u32)dz)) >> 6);
}
