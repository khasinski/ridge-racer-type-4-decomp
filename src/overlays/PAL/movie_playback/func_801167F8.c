/* MASPSX_FLAGS: --assembler-profile=gnu */

/* GCC_VERSION: 2.7.2 */

#include "rr/overlay668_wait.h"
s32 movie_playback_mdec_out_sync(void)
{
    volatile s32 counter = 0x100000;
    while (*(volatile u32 *)movie_playback_mdec_out_dma_chcr & 0x01000000) {
        if (--counter == -1) {
            movie_playback_mdec_timeout(movie_playback_mdec_out_sync_name);
            return -1;
        }
    }
    return 0;
}
