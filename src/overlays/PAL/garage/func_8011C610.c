#include "rr/garage_scale_panel.h"
void garage_draw_preview_window(s32 renderArgument) {
 s32 width=64/(((volatile RrGarageSelectionPrefix*)&garage_logo_state)->config1C>>1);
 s32 height;
 u32 *order;RrGpuScratchPointers *scratch;
 height=128/(((volatile RrGarageSelectionPrefix*)&garage_logo_state)->config1C>>1);
 order=RR_GARAGE_SCRATCH->order+136;
 scratch=RR_GARAGE_SCRATCH;
 scratch->cursor=(u8*)garage_add_rect_outline(order,(RrGpuLineF4Packet*)scratch->cursor,(s16)(garage_logo_state.reset08[0]+237),(s16)(garage_logo_state.reset08[1]*2+71),width,height,0,garage_logo_state.shadeD0,0);
 scratch->cursor=garage_add_draw_area(order,scratch->cursor,12,61,296,352);
 scratch->cursor=(u8*)garage_add_textured_rect(order,(RrGpuPolyFT4Packet*)scratch->cursor,236,69,66,132,71,1,66,66,4272,12);
 scratch->cursor=(u8*)garage_add_textured_rect(order,(RrGpuPolyFT4Packet*)scratch->cursor,236,69,66,132,3,1,66,66,4273,12);
 scratch->cursor=garage_add_draw_area(order,scratch->cursor,237,71,64,128);
 scratch->cursor=(u8*)garage_add_rect_outline(order,(RrGpuLineF4Packet*)scratch->cursor,236,69,66,132,255,255,255);
}
