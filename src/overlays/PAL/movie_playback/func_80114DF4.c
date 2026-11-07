#include "rr/movie_playback_decode.h"
#include "rr/cd.h"
void StUnSetRing(void);
#include "rr/frame_loop.h"
void movie_playback_shutdown(void){
 movie_playback_dec_dct_out_callback(0);
 StUnSetRing();
 CdControlB(9,0,0);
 ModeGroup=1;ModeId=3;ModeSubState=0;
}
