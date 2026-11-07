#include "rr/overlay662_palette.h"
#ifndef RR_GPU_SCRATCH
#define RR_GPU_SCRATCH ((RrGpuScratchPointers *)0x1F800000)
#endif

/* BF04 passes channel0..2 and a masked five-bit component. The tens
 * descriptor is read again after emitting that digit, for the ones position. */
void garage_func_8011C00C(s32 channel, s32 value)
{
    RrGpuScratchPointers *scratch = RR_GPU_SCRATCH;
    s16 sliderX;
    s16 sliderY;
    s16 digitX;
    s16 digitY;
    /* ASM_BARRIER: keep the tens-digit sprite index in $s0 across the first digit call. */
    register s32 tensSprite __asm__("$16");

    switch (channel) {
    case 0:
        sliderX = (value * 2) + 148;
        sliderY = 353;
        digitX = 133;
        digitY = 356;
        break;
    case 1:
        sliderX = (value * 2) + 148;
        sliderY = 371;
        digitX = 133;
        digitY = 374;
        break;
    case 2:
        sliderX = (value * 2) + 148;
        sliderY = 389;
        digitX = 133;
        digitY = 392;
        break;
    }
    scratch->cursor = garage_func_8011B2D4(71, scratch->cursor, sliderX, sliderY);
    tensSprite = (value / 10) + 59;
    /* ASM_BARRIER: prevent CSE from replacing $s0 with the temporary pseudo-register. */
    __asm__("" : "+r"(tensSprite));
    scratch->cursor = garage_func_8011B2D4(tensSprite, scratch->cursor, digitX, digitY);
    scratch->cursor = garage_func_8011B2D4((value % 10) + 59, scratch->cursor, (s16)(digitX + garage_D_80127548[tensSprite]->width04), digitY);
}
