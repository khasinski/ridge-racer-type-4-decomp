#include "rr/overlay662_palette.h"
#ifndef RR_GPU_SCRATCH
#define RR_GPU_SCRATCH ((RrGpuScratchPointers *)0x1F800000)
#endif
void garage_func_8011BF04(void)
{
    RrGpuScratchPointers *scratch=RR_GPU_SCRATCH;
    /* unsigned long retains the complete PAL/native address before selecting a color. */
    RrGaragePaletteEntry *color=(RrGaragePaletteEntry *)(garage_D_801276A8.config22*2 + (unsigned long)garage_D_801276A8.palette48[garage_D_801276A8.active21]);
    /* ASM_BARRIER: retain the captured red component across renderer calls. */
    register u32 red asm("$17");
    u16 blue,green;
    red=color->bytes[0];
    blue=color->value;
    red=(red&31)<<3;
    green=(blue&0x3E0)>>2;
    blue=(blue&0x7C00)>>7;
    scratch->cursor=(u8 *)garage_func_8011B034(scratch->order+139,(RrGpuTilePacket *)scratch->cursor,115,363,9,32,red,green,blue);
    scratch->cursor=garage_func_8011B2D4(70,scratch->cursor,114,361);
    scratch->cursor=garage_func_8011B2D4(46,scratch->cursor,125,356);
    garage_func_8011C00C(0,red>>3);
    garage_func_8011C00C(1,green>>3);
    garage_func_8011C00C(2,blue>>3);
}
