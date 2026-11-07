#include "rr/overlay662_garage.h"
void garage_save_cursor_selection(void){
 RrGarageSelectionPrefix *p=&garage_logo_state;
 s32 a=p->current60,b=p->selectorsB8[0],c=p->selectorsB8[1],d=p->selectorsB8[2],e=p->selectorsB8[3];
 p->saved64[0]=a;p->saved64[1]=b;p->saved64[2]=c;p->saved64[3]=d;p->saved64[4]=e;
}
