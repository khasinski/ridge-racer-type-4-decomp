#include "rr/overlay662_primitives.h"
#include "rr/animated_sprite_grid.h"
#ifndef RR_GPU_SCRATCH
#define RR_GPU_SCRATCH ((RrGpuScratchPointers *)0x1F800000)
#endif
void garage_func_8011B8B4(void)
{
    RrGpuScratchPointers *scratch=RR_GPU_SCRATCH;
    u32 *ordering=scratch->order+132;
    if (!(garage_D_80127668.words[0]&1)) {
        scratch->cursor=garage_func_8011B2D4(garage_D_80127668.words[14],scratch->cursor,
            garage_D_80127668.halves[10],garage_D_80127668.halves[11]);
        scratch->cursor=garage_func_8011ABB4(ordering,scratch->cursor,0,0,320,480);
    }
}
