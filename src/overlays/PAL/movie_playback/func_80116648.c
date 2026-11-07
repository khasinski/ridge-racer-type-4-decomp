/* GCC_VERSION: 2.7.2 */
/* MASPSX_FLAGS: --assembler-profile=gnu */
#include "rr/movie_playback_dma.h"
void movie_playback_mdec_in(const void *command,u32 words)
{
    movie_playback_mdec_in_sync();
    *(volatile u32 *)movie_playback_dma_dpcr |= 0x88;
    *movie_playback_mdec_in_dma_madr = (const u8 *)command + 4;
    *(volatile u32 *)movie_playback_mdec_in_dma_bcr = ((words >> 5) << 16) | 32;
    *(volatile u32 *)movie_playback_mdec_cmd_reg = *(const u32 *)command;
    *(volatile u32 *)movie_playback_mdec_in_dma_chcr = 0x01000201;
}
