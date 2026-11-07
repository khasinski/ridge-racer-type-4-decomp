/* GCC_VERSION: 2.7.2 */
#include "rr/movie_playback_control.h"
s32 movie_playback_dec_dct_in_sync(s32 mode){s32 result;if(mode==0)result=movie_playback_mdec_in_sync();else result=(movie_playback_mdec_status()>>29)&1;return result;}
