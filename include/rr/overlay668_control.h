#ifndef RR_OVERLAY668_CONTROL_H
#define RR_OVERLAY668_CONTROL_H
#include "rr/overlay668_decode.h"

/* Flags bit0 selects the inverse of command bit27; bit1 selects bit25.
 * Submit the updated command using its first halfword as the word count. */
void movie_playback_func_801163FC(RrStrCommandHeader *header,u32 flags);
void movie_playback_func_80116478(void *output,u32 words);
/* Mode0 forwards the blocking helper result. Other modes query status bits
 * 29 and24 respectively. */
s32 movie_playback_func_80116498(s32 mode);
s32 movie_playback_func_801164D4(s32 mode);
RrLibInterruptCallback movie_playback_func_80116510(RrLibInterruptCallback callback);

void movie_playback_func_801166D8(void *output,u32 words);
s32 movie_playback_func_80116764(void);
s32 movie_playback_func_801167F8(void);
u32 movie_playback_func_8011688C(void);
#endif
