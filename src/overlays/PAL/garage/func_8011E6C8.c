#include "rr/overlay662_garage.h"
s32 garage_cursor_outside_rect(s32 raw_x,s32 raw_y,s32 raw_w,s32 raw_h){
 s32 px=garage_logo_work.halves[10];
 s16 x=raw_x,y=raw_y;
 s32 py;
 if(px<x || (py=garage_logo_work.halves[11])<y)return 1;
 return (px<x+(s16)raw_w)?!(py<y+(s16)raw_h):1;
}
