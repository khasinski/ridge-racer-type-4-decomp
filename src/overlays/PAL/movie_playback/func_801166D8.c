/* GCC_VERSION: 2.7.2 */
/* MASPSX_FLAGS: --assembler-profile=gnu */
#include "rr/movie_playback_dma.h"
void movie_playback_mdec_out(void *output,u32 words)
{
    movie_playback_mdec_out_sync();
    *(volatile u32 *)movie_playback_dma_dpcr |= 0x88;
    *(volatile u32 *)movie_playback_mdec_out_dma_chcr = 0;
    *movie_playback_mdec_out_dma_madr = output;
    *(volatile u32 *)movie_playback_mdec_out_dma_bcr = ((words >> 5) << 16) | 32;
    *(volatile u32 *)movie_playback_mdec_out_dma_chcr = 0x01000200;
}
