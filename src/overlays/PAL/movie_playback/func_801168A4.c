/* GCC_VERSION: 2.7.2 */
#include "rr/movie_playback_dma.h"
#include "rr/diagnostics.h"
extern const char movie_playback_mdec_timeout_fmt[];
s32 movie_playback_mdec_timeout(const char *message) {
    /* Preserve the full native register pointer; PAL uses32-bit pointers. */
    u32 *port;
    s32 result;
    DebugPrintStub(movie_playback_mdec_timeout_fmt, message);
    RR_STR_WRITE32(movie_playback_mdec_ctrl_reg, 0x80000000);
    RR_STR_WRITE32(movie_playback_mdec_in_dma_chcr, 0);
    port = movie_playback_mdec_out_dma_chcr;
    RR_STR_WRITE32(port, 0);
    result = 0;
    RR_STR_WRITE32(movie_playback_mdec_ctrl_reg, (RR_STR_READ32(movie_playback_mdec_out_dma_chcr), 0x60000000));
    return result;
}
