#include "rr/overlay668_decode.h"

void movie_playback_stream_init(RrLibInterruptCallback callback) {
    movie_playback_dec_dct_reset(0);
    movie_playback_dec_dct_out_callback(callback);
    StSetRing(StrStreamState->queue20, 32);
    StSetStream(1, 1, -1, 0, 0);
}
