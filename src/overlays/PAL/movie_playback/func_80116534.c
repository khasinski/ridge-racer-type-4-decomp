/* GCC_VERSION: 2.7.2 */
#include "rr/movie_playback_decode.h"
RrLibInterruptCallback movie_playback_dec_dct_out_callback(RrLibInterruptCallback callback){return DMACallback(1,callback);}
