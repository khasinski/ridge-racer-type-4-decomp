#include "rr/movie_playback_camera.h"

void movie_playback_camera_steer(void) {
    s32 unused[4];
    s32 var_a3;
    s32 idx1;
    s32 idx2;
    s32 old_unk462;
    s16 new_unk462;
    s32 diff;
    s32 unk4A0;

    var_a3 = RaceContext.fraction49C;
    idx1 = RaceContext.node498 - ((3072 - var_a3) / 1024);
    idx2 = idx1 - 1;
    if (idx1 < 0) {
        idx1 += TrackNodeCount;
        idx2 = idx1 - 1;
    }
    if (idx2 < 0) {
        idx2 += TrackNodeCount;
    }
    InterpolateLanePointXYZ(&TrackNodes[idx1], &TrackNodes[idx2], 0, (1024 - var_a3) & 0x3FF, RaceContext.target468);
    RaceContext.delta478[0] = (s32)((u32)RaceContext.target468[0] - (u32)(RaceContext.position450[0] / 4));
    RaceContext.delta478[1] = (s32)((u32)RaceContext.target468[1] - (u32)(RaceContext.position450[1] / 4));
    RaceContext.delta478[2] = (s32)((u32)RaceContext.target468[2] - (u32)(RaceContext.position450[2] / 4));
    old_unk462 = RaceContext.yaw462;
    new_unk462 = 1024 - Atan2Table(RaceContext.delta478[0], RaceContext.delta478[2]);
    old_unk462 = (new_unk462 - old_unk462) * 2;
    diff = old_unk462;
    old_unk462 &= 0xFFF;
    RaceContext.yaw462 = new_unk462;
    if (diff & 0x800) {
        old_unk462 |= ~0xFFF;
    }
    unk4A0 = RaceContext.velocity4A0;
    RaceContext.velocity4A0 = (s32)((u32)unk4A0 + (u32)((s32)(((u32)old_unk462 - (u32)unk4A0) * 15U) / 16));
    RaceContext.roll464 = (s32)((u32)RaceContext.velocity4A0 + (u32)RaceContext.roll464) / 2;
}
