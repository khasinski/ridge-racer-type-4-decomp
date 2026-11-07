#include "rr/overlay668_stream.h"
extern s32 CdRead2(s32);

void movie_playback_func_801153F0(u8 *location) {
    u8 mode = 0x80;
    do {
        while (CdControl(2, location, 0) == 0) {
        }
        while (CdControl(14, &mode, 0) == 0) {
        }
        VSync(3);
    } while (CdRead2(480) == 0);
}
