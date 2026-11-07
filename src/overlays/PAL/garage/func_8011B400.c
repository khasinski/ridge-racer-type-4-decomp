#include "rr/overlay662_primitives.h"
#include "rr/animated_sprite_grid.h"
#ifndef RR_GPU_SCRATCH
#define RR_GPU_SCRATCH ((RrGpuScratchPointers *)0x1F800000)
#endif
void garage_func_8011B400(void)
{
    RrGpuScratchPointers *scratch=RR_GPU_SCRATCH;
    u32 *order=scratch->order;
    scratch->cursor=garage_func_8011ABB4(order+146,scratch->cursor,12,61,296,352);
    scratch->cursor=(u8 *)garage_func_8011AC94(order+133,(RrGpuDrawModePacket *)scratch->cursor,704,0);
    scratch->cursor=(u8 *)garage_func_8011AC94(order+147,(RrGpuDrawModePacket *)scratch->cursor,704,0);
}
