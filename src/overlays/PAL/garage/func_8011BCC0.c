#include "rr/overlay662_palette_tile.h"
#ifndef RR_GPU_SCRATCH
#define RR_GPU_SCRATCH ((RrGpuScratchPointers *)0x1F800000)
#endif
void garage_draw_palette_swatch(s32 index)
{
    RrGpuScratchPointers *scratch=RR_GPU_SCRATCH;
    u32 *pal=garage_logo_state.palette48[garage_logo_state.active21];
    s32 slot=index-20;
    RrPaletteEntry16 *color=(RrPaletteEntry16 *)(slot*2+(RrPaletteAddress)pal);
    u32 r=color->bytes[0];
    RrGarageRectangle *rect=garage_logo_buttons[index];
    u16 raw=color->value;
    scratch->cursor=(u8*)garage_add_tile(scratch->order+139,(RrGpuTilePacket*)scratch->cursor,(s16)(*rect)[0],(s16)(*rect)[1],(s16)(*rect)[2],(s16)(*rect)[3],(r&31)<<3,(raw&0x3E0)>>2,(raw&0x7C00)>>7);
    if(slot==garage_logo_state.config22)
        scratch->cursor=garage_add_ui_sprite(69,scratch->cursor,(s16)((*rect)[0]-1),(s16)((*rect)[1]-2));
}
