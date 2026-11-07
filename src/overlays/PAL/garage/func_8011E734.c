#include "rr/overlay662_garage.h"
s32 garage_func_8011E734(s32 index){
 RrGarageRectangle *p=garage_D_801273B8[index];
 s32 px=garage_D_80127668.halves[10],py;
 s32 x=(s16)(*p)[0],y;
 if(px<x || (py=garage_D_80127668.halves[11])<(y=(s16)(*p)[1]))return 1;
 return (px<x+(s16)(*p)[2])?!(py<y+(s16)(*p)[3]):1;
}
