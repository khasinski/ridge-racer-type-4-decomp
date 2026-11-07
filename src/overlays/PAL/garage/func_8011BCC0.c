#include "rr/overlay662_palette_tile.h"
#ifndef RR_GPU_SCRATCH
#define RR_GPU_SCRATCH ((RrGpuScratchPointers *)0x1F800000)
#endif
void garage_func_8011BCC0(s32 index)
{
    RrGpuScratchPointers *scratch=RR_GPU_SCRATCH;
    u32 *pal=garage_D_801276A8.palette48[garage_D_801276A8.active21];
    s32 slot=index-20;
    RrPaletteEntry16 *color=(RrPaletteEntry16 *)(slot*2+(RrPaletteAddress)pal);
    u32 r=color->bytes[0];
    RrGarageRectangle *rect=garage_D_801273B8[index];
    u16 raw=color->value;
    scratch->cursor=(u8*)garage_func_8011B034(scratch->order+139,(RrGpuTilePacket*)scratch->cursor,(s16)(*rect)[0],(s16)(*rect)[1],(s16)(*rect)[2],(s16)(*rect)[3],(r&31)<<3,(raw&0x3E0)>>2,(raw&0x7C00)>>7);
    if(slot==garage_D_801276A8.config22)
        scratch->cursor=garage_func_8011B2D4(69,scratch->cursor,(s16)((*rect)[0]-1),(s16)((*rect)[1]-2));
}
