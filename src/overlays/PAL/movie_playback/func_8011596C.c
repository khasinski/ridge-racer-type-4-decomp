#include "types.h"
s32 movie_playback_func_8011596C(s16 arg0) {
    if (arg0 >= 157) {
        return 0;
    }
    if (arg0 >= 140) {
        return (156 - arg0) << 4;
    }
    if (arg0 < 48) {
        return 0;
    }
    if (arg0 < 65) {
        return (arg0 - 48) << 4;
    }
    return 256;
}
