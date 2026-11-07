#include "rr/garage_image_fade.h"
#ifndef RR_GPU_SCRATCH
#define RR_GPU_SCRATCH ((RrGpuScratchPointers *)0x1F800000U)
#endif
void garage_draw_color_preview_image(s32 mode) {
 RrGarageFadeGeometry quad;
 s32 amount=garage_image_fade_phase;
 s32 high,low,uv;
 s32 i,*x,*y,*u,*v;
 s32 color;
 void *cursor;
 u16 previous;

 s32 fresh;
 if(amount>=129)amount=128;
 i=0;high=256;low=128;uv=63;

 v=quad.v;u=quad.u;y=quad.y;x=quad.x;
 do {
  *x=((i&1)<<6)+128;
  if(i>=2)*y=high;else *y=low;
  *u=(i&1)*63;
  if(i>=2)*v=uv;else *v=0;
  v++;u++;y++;i++;x++;
 }while(i<4);
 if(mode==1){
  amount=rsin(amount*8)/64;
  quad.y[0]+=amount;quad.y[1]=quad.y[0];
  quad.y[2]-=amount;quad.y[3]=quad.y[2];amount=0;
 }
 color=64-rsin((s32)((u32)ScreenFrame*64U))/64;
 cursor=RR_GPU_SCRATCH->cursor;
 if(amount>0){
  cursor=EmitPolyFT3(&CurrentDrawBuf->head70,cursor,quad.x,quad.y,quad.u,quad.v,42,0x1028,0,(u16)amount);
  cursor=EmitPolyFT3(&CurrentDrawBuf->head70,cursor,quad.x+1,quad.y+1,quad.u+1,quad.v+1,42,0x1028,0,(u16)amount);
  cursor=AddTilePrim(&CurrentDrawBuf->head70,cursor,quad.x[0],quad.y[0],(s32)((u32)quad.x[1]-(u32)quad.x[0]),(s32)((u32)quad.y[2]-(u32)quad.y[0]),color,color,color);
 }else cursor=AddTilePrim(&CurrentDrawBuf->head70,cursor,quad.x[0],quad.y[0],(s32)((u32)quad.x[1]-(u32)quad.x[0]),(s32)((u32)quad.y[2]-(u32)quad.y[0]),0,0,0);
 fresh=garage_image_fade_phase;

 previous=(u16)garage_image_fade_phase;
 RR_GPU_SCRATCH->cursor=cursor;

 if(fresh>=128)garage_image_fade_phase=128;
 else {
  garage_image_fade_phase=previous+4;
  if(mode==1){
   /* Empty compiler barrier preserves the conditional phase update. */
   __asm__("" : : : "memory");
   garage_image_fade_phase=previous+8;
  }
 }
}
