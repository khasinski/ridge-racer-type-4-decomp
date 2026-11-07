#include "rr/movie_playback_camera.h"

void movie_playback_camera_apply(void) {
    RrPreviewMatrixWorkspace sp10;
    RrPreviewVector8 sp30;
    s32 *scratch = RR_REPLAY_SCRATCH_WORDS;

    scratch[6] = RaceContext.pitch460;
    scratch[7] = RaceContext.yaw462;
    scratch[8] = RaceContext.roll464;
    scratch[2] = RaceContext.position450[0];
    scratch[3] = RaceContext.position450[1];
    scratch[4] = RaceContext.position450[2];
    sp30.x = scratch[6];
    sp30.y = scratch[7];
    sp30.z = scratch[8];
    RotMatrix(&sp30.x, sp10.halves);
    ApplyMatrixSV(&sp10.matrix, RaceContext.offset488, &sp30);
    PlayerCars[0].pos20[0] = (RaceContext.position450[0] / 4) + sp30.x;
    PlayerCars[0].pos20[1] = (RaceContext.position450[1] / 4) + sp30.y;
    PlayerCars[0].pos20[2] = (RaceContext.position450[2] / 4) + sp30.z;
    PlayerCars[0].rot50 = RaceContext.pitch460 + RaceContext.angle490;
    PlayerCars[0].rot54 = RaceContext.yaw462;
    PlayerCars[0].rot58 = RaceContext.roll464;
    BuildCameraMatrices();
}
