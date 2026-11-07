#include "rr/garage_garage.h"
void garage_set_clip_from_button(s32 index){
 RrGarageRectangle *p=garage_logo_buttons[index];
 garage_logo_work.halves[24]=(*p)[0];
 garage_logo_work.halves[25]=(*p)[1];
 garage_logo_work.halves[26]=(*p)[2];
 garage_logo_work.halves[27]=(*p)[3];
}
