#include "rr/garage_garage.h"
s32 garage_cursor_outside_button(s32 index){
 RrGarageRectangle *p=garage_logo_buttons[index];
 s32 px=garage_logo_work.halves[10],py;
 s32 x=(s16)(*p)[0],y;
 if(px<x || (py=garage_logo_work.halves[11])<(y=(s16)(*p)[1]))return 1;
 return (px<x+(s16)(*p)[2])?!(py<y+(s16)(*p)[3]):1;
}
