#include "rr/overlay668_frame_submit.h"

s32 ovl668_func_801150D8(RrStrDisplayConfig *config) {
    register u8 *data __asm__("$16");
    register s32 remaining __asm__("$17") = 2000;
    u32 *index;

    do {
        data = ovl668_func_80115164(config);
        if (data != 0) {
            goto found;
        }
    } while (--remaining != 0);
    return -1;

found:
    index = &config->word00;
    *index = *index < 1U;
    ovl668_func_80116958(data, StrStreamState->commands10[*index]);
    return StFreeRing(data);
}
