/* GCC_VERSION: 2.7.2 */
/* MASPSX_FLAGS: --assembler-profile=gnu */
#include "rr/overlay668_dma.h"
#include "rr/diagnostics.h"
extern const char movie_playback_D_801148C0[];

void movie_playback_func_80116558(s32 mode) {
    switch (mode) {
    case 0:
        RR_STR_WRITE32(movie_playback_D_80116E54, 0x80000000);
        RR_STR_WRITE32(movie_playback_D_80116E28, 0);
        RR_STR_WRITE32(movie_playback_D_80116E34, 0);
        RR_STR_WRITE32(movie_playback_D_80116E54, 0x60000000);
        movie_playback_func_80116648(&movie_playback_D_80116D10, 32);
        movie_playback_func_80116648(&movie_playback_D_80116D94, 32);
        break;
    case 1:
        RR_STR_WRITE32(movie_playback_D_80116E54, 0x80000000);
        RR_STR_WRITE32(movie_playback_D_80116E28, 0);
        RR_STR_WRITE32(movie_playback_D_80116E34, 0);
        RR_STR_WRITE32(movie_playback_D_80116E54, (RR_STR_READ32(movie_playback_D_80116E34), 0x60000000));
        break;
    default:
        DebugPrintStub(movie_playback_D_801148C0, mode);
        break;
    }
}
