/* GCC_VERSION: 2.7.2 */
/* MASPSX_FLAGS: --assembler-profile=gnu */
#include "rr/movie_playback_dma.h"
#include "rr/diagnostics.h"
extern const char movie_playback_mdec_reset_bad_option_fmt[];

void movie_playback_mdec_reset(s32 mode) {
    switch (mode) {
    case 0:
        RR_STR_WRITE32(movie_playback_mdec_ctrl_reg, 0x80000000);
        RR_STR_WRITE32(movie_playback_mdec_in_dma_chcr, 0);
        RR_STR_WRITE32(movie_playback_mdec_out_dma_chcr, 0);
        RR_STR_WRITE32(movie_playback_mdec_ctrl_reg, 0x60000000);
        movie_playback_mdec_in(&movie_playback_dct_quant_command, 32);
        movie_playback_mdec_in(&movie_playback_dct_matrix_command, 32);
        break;
    case 1:
        RR_STR_WRITE32(movie_playback_mdec_ctrl_reg, 0x80000000);
        RR_STR_WRITE32(movie_playback_mdec_in_dma_chcr, 0);
        RR_STR_WRITE32(movie_playback_mdec_out_dma_chcr, 0);
        RR_STR_WRITE32(movie_playback_mdec_ctrl_reg, (RR_STR_READ32(movie_playback_mdec_out_dma_chcr), 0x60000000));
        break;
    default:
        DebugPrintStub(movie_playback_mdec_reset_bad_option_fmt, mode);
        break;
    }
}
