#include "rr/overlay662_buttons.h"
#ifndef RR_GPU_SCRATCH
#define RR_GPU_SCRATCH ((RrGpuScratchPointers *)0x1F800000)
#endif

/* The mode cycle established by1A788/1A414 and1FB2C is0..2. */
void garage_func_8011BDC8(s32 index)
{
    RrGpuScratchPointers *scratch = RR_GPU_SCRATCH;
    RrGarageButton *button = (RrGarageButton *)garage_D_801273B8[index];
    s32 sprite0;
    s32 sprite1;

    if (button->timer0E == 0) {
        sprite0 = 16;
        switch (garage_D_801276A8.config1F) {
        case 0:
            sprite1 = 18;
            break;
        case 1:
            sprite1 = 20;
            break;
        case 2:
            sprite1 = 22;
            break;
        }
    } else {
        sprite0 = 17;
        switch (garage_D_801276A8.config1F) {
        case 0:
            sprite1 = 19;
            break;
        case 1:
            sprite1 = 21;
            break;
        case 2:
            sprite1 = 23;
            break;
        }
        if (button->timer0E > 0) {
            button->timer0E--;
        }
    }
    scratch->cursor = garage_func_8011B2D4(sprite0, scratch->cursor, 19, 173);
    scratch->cursor = garage_func_8011B2D4(sprite1, scratch->cursor, 39, 173);
}
