#include "rr/overlay662_garage.h"
void PlaySfx(s32 mode);
void garage_func_801256D4(void)
{
    if(garage_D_80127668.words[1]&2){
        PlaySfx(1);
        garage_D_801276A8.active21=!garage_D_801276A8.active21;
    }
}
