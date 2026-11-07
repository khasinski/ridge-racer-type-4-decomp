#ifndef RR_OVERLAY668_CONTROL_H
#define RR_OVERLAY668_CONTROL_H
#include "rr/overlay668_decode.h"

/* Flags bit0 selects the inverse of command bit27; bit1 selects bit25.
 * Submit the updated command using its first halfword as the word count. */
void movie_playback_dec_dct_in(RrStrCommandHeader *header,u32 flags);
void movie_playback_dec_dct_out(void *output,u32 words);
/* Mode0 forwards the blocking helper result. Other modes query status bits
 * 29 and24 respectively. */
s32 movie_playback_dec_dct_in_sync(s32 mode);
s32 movie_playback_dec_dct_out_sync(s32 mode);
RrLibInterruptCallback movie_playback_dec_dct_in_callback(RrLibInterruptCallback callback);

void movie_playback_mdec_out(void *output,u32 words);
s32 movie_playback_mdec_in_sync(void);
s32 movie_playback_mdec_out_sync(void);
u32 movie_playback_mdec_status(void);
#endif
