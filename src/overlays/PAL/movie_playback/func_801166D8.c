/* GCC_VERSION: 2.7.2 */
/* MASPSX_FLAGS: --assembler-profile=gnu */
#include "rr/overlay668_dma.h"
void movie_playback_func_801166D8(void *output,u32 words)
{
    movie_playback_func_801167F8();
    *(volatile u32 *)movie_playback_D_80116E58 |= 0x88;
    *(volatile u32 *)movie_playback_D_80116E34 = 0;
    *movie_playback_D_80116E2C = output;
    *(volatile u32 *)movie_playback_D_80116E30 = ((words >> 5) << 16) | 32;
    *(volatile u32 *)movie_playback_D_80116E34 = 0x01000200;
}
