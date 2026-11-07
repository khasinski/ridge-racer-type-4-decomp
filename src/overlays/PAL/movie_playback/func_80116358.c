#include "rr/overlay668_decode.h"
u32 *movie_playback_dec_dct_put_env(u32 *input){
 u32 *original=input;
 register u32 *source=input;
 /* ASM_BARRIER: Preserve the loop destination register until both copies end. */
 register u32 *destination __asm__("$5")=movie_playback_dct_quant_command.luma;
 s32 n;
 for(n=15;n!=-1;n--)*destination++=*source++;
 destination=movie_playback_dct_quant_command.chroma;source=original+16;
 for(n=15;n!=-1;n--)*destination++=*source++;
 movie_playback_mdec_in(&movie_playback_dct_quant_command,32);
 movie_playback_mdec_in(&movie_playback_dct_matrix_command,32);
 return original;
}
