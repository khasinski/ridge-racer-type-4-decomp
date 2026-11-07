#include "rr/overlay662_garage.h"
void garage_func_8011D120(void)
{
    s32 i;
    u8 selected=garage_D_801276A8.active21;
    u8 other;
    u32 *source,*destination;
    garage_D_801276A8.dirtyD3=1;
    other=!selected;
    source=garage_D_801276A8.image38[selected];
    destination=garage_D_801276A8.image38[other];
    for(i=0;i<512;){i++;*destination++=*source++;}
    source=garage_D_801276A8.image40[selected];
    destination=garage_D_801276A8.image40[other];
    for(i=0;i<512;){i++;*destination++=*source++;}
    source=garage_D_801276A8.palette48[selected];
    destination=garage_D_801276A8.palette48[other];
    for(i=0;i<8;){i++;*destination++=*source++;}
}
