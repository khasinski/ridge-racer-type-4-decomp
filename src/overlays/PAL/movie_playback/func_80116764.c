/* MASPSX_FLAGS: --assembler-profile=gnu */

/* GCC_VERSION: 2.7.2 */

#include "rr/overlay668_wait.h"
s32 movie_playback_func_80116764(void)
{
    volatile s32 counter = 0x100000;
    while (*(volatile u32 *)movie_playback_D_80116E54 & 0x20000000) {
        if (--counter == -1) {
            movie_playback_func_801168A4(movie_playback_D_801148DC);
            return -1;
        }
    }
    return 0;
}
