#include "rr/overlay668_camera.h"

void movie_playback_func_80115DFC(void) {
    s32 i;
    RrTrackSample3C *p;

    SetDispMask(0);
    SetupDrawDispEnvs(0, 0, 0, 1);
    VSyncThreshold = 384;
    InitViewState();
    SetupRenderer(5);
    BuildCameraMatrices();
    p = TrackNodes;
    RaceContext.counter444 = 0;
    RaceContext.state44C = 0;
    RaceContext.counter4A4 = 0;
    RaceContext.mode4A8 = 1;
    RaceContext.phase4AC = 0;
    RaceContext.position450[0] = (s32)((u32)p[320].x08 << 2);
    RaceContext.position450[1] = p[320].y10 - 120;
    RaceContext.position450[2] = (s32)((u32)p[320].z0C << 2);
    RaceContext.pitch460 = 0;
    RaceContext.yaw462 = (s16)(0U-(u32)(u16)p[320].heading12);
    RaceContext.roll464 = 0;
    RaceContext.offset488[0] = 0;
    RaceContext.offset488[1] = 0;
    RaceContext.offset488[2] = 0;
    RaceContext.angle490 = -96;
    ViewUsesCarTrackNode = 0;
    RaceContext.timer492 = 0;
    RaceContext.timer494 = 0;
    RaceContext.node498 = 320;
    movie_playback_func_80115460(RaceContext.position450);
    for (i = 0; i < 16; i++) {
        movie_playback_func_80115554();
    }
    movie_playback_func_801156FC();
    movie_playback_func_80115850();
    ResetScreenCopyOverlay();
    SetSceneColorParams(0, 144, 144, 144);
    SetSceneOverlayEnabled(1);
    MaskBitSetting = 1;
    D_80127E90 = 16000;
    D_80127E94 = movie_playback_D_80116CC0[(s32)((u32)GameLanguage * 4U + (u32)(s32)SelectedTeamIndex)];
}
