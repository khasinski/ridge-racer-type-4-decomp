#include "rr/overlay662_scroll_panel.h"
#ifndef RR_GPU_SCRATCH
#define RR_GPU_SCRATCH ((RrGpuScratchPointers *)0x1F800000U)
#endif
void garage_func_8011C37C(s32 mode) {
 RrGpuScratchPointers *scratch=RR_GPU_SCRATCH;
 u32 *ordering=scratch->order+140;
 RrGpuRectangleFields rectangle;
 switch(mode) {
 case 46:
  garage_func_8011C174(0,&rectangle);
  scratch->cursor=(u8 *)garage_func_8011B034(ordering,(RrGpuTilePacket *)scratch->cursor,rectangle.x,rectangle.y,rectangle.w,rectangle.h,204,204,204);
  scratch->cursor=(u8 *)garage_func_8011B034(ordering,(RrGpuTilePacket *)scratch->cursor,90,337,112,8,77,77,77);
  scratch->cursor=(u8 *)garage_func_8011AEC4(ordering,(RrGpuLineF4Packet *)scratch->cursor,89,335,114,12,136,136,136);
  return;
 case 47:
  garage_func_8011C174(1,&rectangle);
  scratch->cursor=(u8 *)garage_func_8011B034(ordering,(RrGpuTilePacket *)scratch->cursor,rectangle.x,rectangle.y,rectangle.w,rectangle.h,204,204,204);
  scratch->cursor=(u8 *)garage_func_8011B034(ordering,(RrGpuTilePacket *)scratch->cursor,216,87,4,224,77,77,77);
  scratch->cursor=(u8 *)garage_func_8011AEC4(ordering,(RrGpuLineF4Packet *)scratch->cursor,215,85,6,228,136,136,136);
  return;
 case 48:
  scratch->cursor=(u8 *)garage_func_8011B0CC(ordering,(RrPolyG4Storage *)scratch->cursor,149,355,65,13,0,0,0,255,0,0);return;
 case 49:
  scratch->cursor=(u8 *)garage_func_8011B0CC(ordering,(RrPolyG4Storage *)scratch->cursor,149,373,65,13,0,0,0,0,255,0);return;
 case 50:
  scratch->cursor=(u8 *)garage_func_8011B0CC(ordering,(RrPolyG4Storage *)scratch->cursor,149,391,65,13,0,0,0,0,0,255);return;
 }
}
