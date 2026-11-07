/* GCC_VERSION: 2.7.2 */
#include "rr/overlay668_decode.h"

u32 *movie_playback_dec_dct_get_env(u32 *output){
 u32 *cursor=output,*source=movie_playback_dct_quant_command.luma;
 s32 n;
 for(n=15;n!=-1;n--)*cursor++=*source++;
 cursor=output+16;source=movie_playback_dct_quant_command.chroma;
 for(n=15;n!=-1;n--)*cursor++=*source++;
 cursor=output+32;source=movie_playback_dct_matrix_command.matrix;
 for(n=31;n!=-1;n--)*cursor++=*source++;
 return output;
}
