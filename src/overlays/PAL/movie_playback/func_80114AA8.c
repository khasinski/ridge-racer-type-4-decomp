#include "rr/overlay668_dispatch.h"
void movie_playback_update(void) {
    switch (RaceContextMoviePhase) {
    case 0:
        movie_playback_start(LoadCursor);
        break;
    case 1:
        movie_playback_step();
        break;
    case 2:
        movie_playback_shutdown();
        break;
    }
}
