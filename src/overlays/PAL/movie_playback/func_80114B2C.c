#include "rr/overlay668_stream.h"
#define STR_STATE ((RrStrPlaybackState *)RaceContext.activeB8)

void movie_playback_func_80114B2C(u8 *arg0) {
    s32 vol;
    SetDispMask(0);
    STR_STATE->width00 = 320;
    STR_STATE->height04 = 240;
    SsSetSerialAttr(0, 0, 1);
    vol = (OptionsMusicVolume * 127) / 16;
    SsSetSerialVol(0, vol, vol);
    VSyncThreshold = 128;
    movie_playback_func_80114E48(arg0);
    movie_playback_func_80114E90(&STR_STATE->display24, 0, 24, 0, 264);
    movie_playback_func_80114EE8(movie_playback_func_80114F48);
    while (movie_playback_func_801150D8(&STR_STATE->display24) == -1) {
        movie_playback_func_801153F0(RaceContext.currentC0.c);
    }
    RaceContext.phaseB4 = 1;
    RaceContext.counterC4 = 0;
    ModeFrame = 0;
    VSync(0);
}
