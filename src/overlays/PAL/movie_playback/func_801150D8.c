#include "rr/movie_playback_frame_submit.h"

s32 movie_playback_next_vlc(RrStrDisplayConfig *config) {
    register u8 *data __asm__("$16");
    register s32 remaining __asm__("$17") = 2000;
    u32 *index;

    do {
        data = movie_playback_next_frame(config);
        if (data != 0) {
            goto found;
        }
    } while (--remaining != 0);
    return -1;

found:
    index = &config->word00;
    *index = *index < 1U;
    movie_playback_dec_dct_vlc(data, StrStreamState->commands10[*index]);
    return StFreeRing(data);
}
