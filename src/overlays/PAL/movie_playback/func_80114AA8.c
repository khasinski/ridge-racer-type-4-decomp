#include "rr/overlay668_dispatch.h"
void movie_playback_func_80114AA8(void) {
    switch (RaceContextMoviePhase) {
    case 0:
        movie_playback_func_80114B2C(LoadCursor);
        break;
    case 1:
        movie_playback_func_80114C3C();
        break;
    case 2:
        movie_playback_func_80114DF4();
        break;
    }
}
