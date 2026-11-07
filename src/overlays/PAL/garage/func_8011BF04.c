#include "rr/overlay662_palette.h"
#ifndef RR_GPU_SCRATCH
#define RR_GPU_SCRATCH ((RrGpuScratchPointers *)0x1F800000)
#endif
void garage_draw_color_panel(void)
{
    RrGpuScratchPointers *scratch=RR_GPU_SCRATCH;
    /* unsigned long retains the complete PAL/native address before selecting a color. */
    RrGaragePaletteEntry *color=(RrGaragePaletteEntry *)(garage_logo_state.config22*2 + (unsigned long)garage_logo_state.palette48[garage_logo_state.active21]);
    /* ASM_BARRIER: retain the captured red component across renderer calls. */
    register u32 red asm("$17");
    u16 blue,green;
    red=color->bytes[0];
    blue=color->value;
    red=(red&31)<<3;
    green=(blue&0x3E0)>>2;
    blue=(blue&0x7C00)>>7;
    scratch->cursor=(u8 *)garage_add_tile(scratch->order+139,(RrGpuTilePacket *)scratch->cursor,115,363,9,32,red,green,blue);
    scratch->cursor=garage_add_ui_sprite(70,scratch->cursor,114,361);
    scratch->cursor=garage_add_ui_sprite(46,scratch->cursor,125,356);
    garage_draw_color_channel(0,red>>3);
    garage_draw_color_channel(1,green>>3);
    garage_draw_color_channel(2,blue>>3);
}
