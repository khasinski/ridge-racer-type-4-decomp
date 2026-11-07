#include "rr/overlay668_decode.h"
#include "rr/cd.h"
void StUnSetRing(void);
#include "rr/frame_loop.h"
void movie_playback_func_80114DF4(void){
 movie_playback_func_80116534(0);
 StUnSetRing();
 CdControlB(9,0,0);
 ModeGroup=1;ModeId=3;ModeSubState=0;
}
