/* GCC_VERSION: 2.7.2 */
#include "rr/movie_playback_control.h"
RrLibInterruptCallback movie_playback_dec_dct_in_callback(RrLibInterruptCallback callback){return DMACallback(0,callback);}
