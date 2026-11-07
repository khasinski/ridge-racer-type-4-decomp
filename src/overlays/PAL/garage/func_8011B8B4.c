#include "rr/overlay662_primitives.h"
#include "rr/animated_sprite_grid.h"
#ifndef RR_GPU_SCRATCH
#define RR_GPU_SCRATCH ((RrGpuScratchPointers *)0x1F800000)
#endif
void garage_draw_cursor_sprite(void)
{
    RrGpuScratchPointers *scratch=RR_GPU_SCRATCH;
    u32 *ordering=scratch->order+132;
    if (!(garage_logo_work.words[0]&1)) {
        scratch->cursor=garage_add_ui_sprite(garage_logo_work.words[14],scratch->cursor,
            garage_logo_work.halves[10],garage_logo_work.halves[11]);
        scratch->cursor=garage_add_draw_area(ordering,scratch->cursor,0,0,320,480);
    }
}
