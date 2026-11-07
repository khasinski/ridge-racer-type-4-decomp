#include "rr/overlay668_stream.h"
#define STR_STATE ((RrStrPlaybackState *)RaceContext.activeB8)

void movie_playback_start(u8 *arg0) {
    s32 vol;
    SetDispMask(0);
    STR_STATE->width00 = 320;
    STR_STATE->height04 = 240;
    SsSetSerialAttr(0, 0, 1);
    vol = (OptionsMusicVolume * 127) / 16;
    SsSetSerialVol(0, vol, vol);
    VSyncThreshold = 128;
    movie_playback_set_buffers(arg0);
    movie_playback_set_dec_env(&STR_STATE->display24, 0, 24, 0, 264);
    movie_playback_stream_init(movie_playback_dct_out_callback);
    while (movie_playback_next_vlc(&STR_STATE->display24) == -1) {
        movie_playback_kick_cd(RaceContext.currentC0.c);
    }
    RaceContext.phaseB4 = 1;
    RaceContext.counterC4 = 0;
    ModeFrame = 0;
    VSync(0);
}
