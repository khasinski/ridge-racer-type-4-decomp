#include "rr/garage_scroll_panel.h"
#ifndef RR_GPU_SCRATCH
#define RR_GPU_SCRATCH ((RrGpuScratchPointers *)0x1F800000U)
#endif
void garage_draw_slider_track(s32 mode) {
 RrGpuScratchPointers *scratch=RR_GPU_SCRATCH;
 u32 *ordering=scratch->order+140;
 RrGpuRectangleFields rectangle;
 switch(mode) {
 case 46:
  garage_get_scrollbar_thumb(0,&rectangle);
  scratch->cursor=(u8 *)garage_add_tile(ordering,(RrGpuTilePacket *)scratch->cursor,rectangle.x,rectangle.y,rectangle.w,rectangle.h,204,204,204);
  scratch->cursor=(u8 *)garage_add_tile(ordering,(RrGpuTilePacket *)scratch->cursor,90,337,112,8,77,77,77);
  scratch->cursor=(u8 *)garage_add_rect_outline(ordering,(RrGpuLineF4Packet *)scratch->cursor,89,335,114,12,136,136,136);
  return;
 case 47:
  garage_get_scrollbar_thumb(1,&rectangle);
  scratch->cursor=(u8 *)garage_add_tile(ordering,(RrGpuTilePacket *)scratch->cursor,rectangle.x,rectangle.y,rectangle.w,rectangle.h,204,204,204);
  scratch->cursor=(u8 *)garage_add_tile(ordering,(RrGpuTilePacket *)scratch->cursor,216,87,4,224,77,77,77);
  scratch->cursor=(u8 *)garage_add_rect_outline(ordering,(RrGpuLineF4Packet *)scratch->cursor,215,85,6,228,136,136,136);
  return;
 case 48:
  scratch->cursor=(u8 *)garage_add_gradient_rect(ordering,(RrPolyG4Storage *)scratch->cursor,149,355,65,13,0,0,0,255,0,0);return;
 case 49:
  scratch->cursor=(u8 *)garage_add_gradient_rect(ordering,(RrPolyG4Storage *)scratch->cursor,149,373,65,13,0,0,0,0,255,0);return;
 case 50:
  scratch->cursor=(u8 *)garage_add_gradient_rect(ordering,(RrPolyG4Storage *)scratch->cursor,149,391,65,13,0,0,0,0,0,255);return;
 }
}
