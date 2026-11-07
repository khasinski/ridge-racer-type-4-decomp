/* GCC_VERSION: 2.7.2 */
#include "rr/overlay668_dma.h"
#include "rr/diagnostics.h"
extern const char movie_playback_D_801148FC[];
s32 movie_playback_func_801168A4(const char *message) {
    /* Preserve the full native register pointer; PAL uses32-bit pointers. */
    u32 *port;
    s32 result;
    DebugPrintStub(movie_playback_D_801148FC, message);
    RR_STR_WRITE32(movie_playback_D_80116E54, 0x80000000);
    RR_STR_WRITE32(movie_playback_D_80116E28, 0);
    port = movie_playback_D_80116E34;
    RR_STR_WRITE32(port, 0);
    result = 0;
    RR_STR_WRITE32(movie_playback_D_80116E54, (RR_STR_READ32(movie_playback_D_80116E34), 0x60000000));
    return result;
}
