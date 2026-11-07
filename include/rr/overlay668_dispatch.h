#ifndef RR_OVERLAY668_DISPATCH_H
#define RR_OVERLAY668_DISPATCH_H
#include "rr/menu_choice.h"
#include "rr/overlay668_decode.h"
/* Current playback phase:0 starts from the published resource buffer,
 *1 updates playback,2 shuts it down. Other signed word values are idle. */
extern s32 D_800AC704;
void movie_playback_func_80114AA8(void);
void movie_playback_func_80114B2C(u8 *resource);
void movie_playback_func_80114C3C(void);
#endif
