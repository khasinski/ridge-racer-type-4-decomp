#include "rr/overlay668_display.h"
void movie_playback_set_dec_env(RrStrDisplayConfig*p,s32 x,s32 y,s32 second_x,s32 second_y){
 p->rectangles[0].w=480;
 p->rectangles[1].w=480;
 p->rectangles[0].x=x;
 p->rectangles[0].y=y;
 p->rectangles[0].h=240;
 p->rectangles[1].x=second_x;
 p->rectangles[1].h=240;
 p->rectangles[2].x=x;
 p->rectangles[2].y=y;
 p->rectangles[2].w=24;
 p->rectangles[2].h=240;
 p->rectangles[1].y=second_y;
 p->alternate04=~(u32)DrawBufIndex&1;
}
