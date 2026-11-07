#include "rr/garage_primitives.h"
#include "rr/animated_sprite_grid.h"
#ifndef RR_GPU_SCRATCH
#define RR_GPU_SCRATCH ((RrGpuScratchPointers *)0x1F800000)
#endif
void garage_add_editor_draw_env(void)
{
    RrGpuScratchPointers *scratch=RR_GPU_SCRATCH;
    u32 *order=scratch->order;
    scratch->cursor=garage_add_draw_area(order+146,scratch->cursor,12,61,296,352);
    scratch->cursor=(u8 *)garage_add_draw_mode(order+133,(RrGpuDrawModePacket *)scratch->cursor,704,0);
    scratch->cursor=(u8 *)garage_add_draw_mode(order+147,(RrGpuDrawModePacket *)scratch->cursor,704,0);
}
