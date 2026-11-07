#include "rr/overlay662_actions.h"
void garage_func_8011DCDC(u32 flags){
 if(flags)PlaySfx(0);
 if(flags&1)garage_D_80127470[garage_D_801276A8.current60]->callback(1);
 else if(flags&2)garage_D_80127470[garage_D_801276A8.current60]->callback(2);
 if(flags&4)garage_D_80127470[garage_D_801276A8.current60]->callback(4);
 else if(flags&8)garage_D_80127470[garage_D_801276A8.current60]->callback(8);
}
