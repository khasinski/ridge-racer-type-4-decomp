#ifndef RR_OVERLAY668_FRAME_SUBMIT_H
#define RR_OVERLAY668_FRAME_SUBMIT_H
#include "rr/overlay668_frame_acquire.h"
#include "psyq/libcd/dsstrm3.h"
void movie_playback_func_80116958(u8 *data,void *commands);
/* Poll at most2000 times. Success toggles the current fullword selector to0/1,
 * captures that selector, reads the fresh stream root and submits the acquired
 * full data pointer with its command buffer. Release status is returned intact;
 * exhausted polling returns-1 without touching command storage or releasing.
 * Acquire may mutate the complete display config and replace the stream root.
 */
s32 movie_playback_func_801150D8(RrStrDisplayConfig *config);
#endif
