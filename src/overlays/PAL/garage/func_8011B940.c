#include "rr/overlay662_buttons.h"
#ifndef RR_GPU_SCRATCH
#define RR_GPU_SCRATCH ((RrGpuScratchPointers *)0x1F800000)
#endif
void garage_func_8011B940(s32 index)
{
    RrGpuScratchPointers *scratch=RR_GPU_SCRATCH;
    RrGarageButton *button=(RrGarageButton *)garage_D_801273B8[index];
    s16 timer=button->timer0E;
    s32 sprite;
    if(timer==0)sprite=button->sprite08;
    else {
        sprite=button->sprite08+1;
        if(timer>0)button->timer0E=timer-1;
    }
    scratch->cursor=garage_func_8011B2D4(sprite,scratch->cursor,(s16)button->rectangle[0],(s16)button->rectangle[1]);
}
