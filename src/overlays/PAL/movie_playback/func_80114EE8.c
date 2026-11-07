#include "rr/overlay668_decode.h"

void movie_playback_func_80114EE8(RrLibInterruptCallback callback) {
    movie_playback_func_80116298(0);
    movie_playback_func_80116534(callback);
    StSetRing(StrStreamState->queue20, 32);
    StSetStream(1, 1, -1, 0, 0);
}
