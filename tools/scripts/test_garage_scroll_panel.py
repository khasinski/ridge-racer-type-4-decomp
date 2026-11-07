"""Scrollbar/gradient orchestration with full native roots and service mutation."""
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess,unittest
class ScrollPanelTest(unittest.TestCase):
 def test_dispatch_and_fresh_cursors(self):
  root=Path(__file__).resolve().parents[2]
  body=(root/'src/overlays/PAL/garage/func_8011C37C.c').read_text()
  source=r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4||sizeof(t)==n,"PAL size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4||offsetof(t,f)==n,"PAL offset")
#include "rr/overlay662_scroll_panel.h"
static struct World {u32 guard[4];RrGpuScratchPointers scratch;u32 orders[3][256];u32 arena[256];u32 tail[4];} world,expected;
static struct Event {unsigned kind;u32 args[10];u8 *cursor;struct World before;} events[4];
static unsigned count,seen,scenario;static int reference_run;static u32 *captured;
static RrGpuRectangleFields rectangle;
#define RR_GPU_SCRATCH (&world.scratch)
'''+body+r'''
static u32 seed=0x1c37c;static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static s16 half(u32 n){u16 v=(u16)n;s16 r;memcpy(&r,&v,2);return r;}
static unsigned offset(const void *p){uintptr_t a=(uintptr_t)p,b=(uintptr_t)world.arena;assert(a>=b&&a<b+sizeof(world.arena));return (unsigned)(a-b);}
static void effect(struct World *w,unsigned kind,const u32 *args,u8 *cursor){
 if(kind==0){w->scratch.cursor=(u8*)world.arena+64;w->scratch.order=world.orders[1];w->orders[0][140]^=0xABCDEF01U;return;}
 unsigned n=offset(cursor),length=kind==1?sizeof(RrGpuTilePacket):kind==2?sizeof(RrGpuLineF4Packet):sizeof(RrPolyG4Storage);
 assert(n+length<sizeof(w->arena));for(unsigned i=0;i<length;i++)((u8*)w->arena)[n+i]^=(u8)(args[i%10]+i+kind);
 w->orders[0][140]^=0xA0000000U|kind;w->scratch.cursor=(u8*)world.arena+768;w->scratch.order=world.orders[2];
}
static void service(unsigned kind,u32 *ordering,u8 *cursor,const u32 *args){
 struct World *w=reference_run?&expected:&world;
 if(kind){assert(ordering==captured);assert((uintptr_t)cursor>UINT32_MAX);}
 if(reference_run){assert(count<4);events[count].kind=kind;events[count].cursor=cursor;memcpy(events[count].args,args,40);events[count++].before=*w;}
 else{assert(seen<count);struct Event *e=&events[seen++];assert(e->kind==kind&&e->cursor==cursor&&memcmp(e->args,args,40)==0&&memcmp(&world,&e->before,sizeof(world))==0);}
 effect(w,kind,args,cursor);
}
void garage_get_scrollbar_thumb(s32 mode,RrGpuRectangleFields *out){u32 args[10]={(u32)mode};service(0,0,0,args);*out=rectangle;}
RrGpuTilePacket *garage_add_tile(u32 *o,RrGpuTilePacket *p,u32 x,u32 y,u32 w,u32 h,u32 r,u32 g,u32 b){u32 a[10]={x,y,w,h,r,g,b};service(1,o,(u8*)p,a);return (RrGpuTilePacket*)((u8*)p+sizeof(*p));}
RrGpuLineF4Packet *garage_add_rect_outline(u32 *o,RrGpuLineF4Packet *p,u32 x,u32 y,u32 w,u32 h,u32 r,u32 g,u32 b){u32 a[10]={x,y,w,h,r,g,b};service(2,o,(u8*)p,a);return (RrGpuLineF4Packet*)((u8*)p+sizeof(*p));}
RrPolyG4Storage *garage_add_gradient_rect(u32 *o,RrPolyG4Storage *p,u32 x,u32 y,u32 w,u32 h,u32 r,u32 g,u32 b,u32 r1,u32 g1,u32 b1){u32 a[10]={x,y,w,h,r,g,b,r1,g1,b1};service(3,o,(u8*)p,a);return (RrPolyG4Storage*)((u8*)p+sizeof(*p));}
static void emit(unsigned kind,u32 *args){u8 *cursor=expected.scratch.cursor;service(kind,captured,cursor,args);expected.scratch.cursor=cursor+(kind==1?sizeof(RrGpuTilePacket):kind==2?sizeof(RrGpuLineF4Packet):sizeof(RrPolyG4Storage));}
static void reference(s32 mode){
 captured=world.orders[0]+140;
 if(mode==46||mode==47){
  u32 arg[10]={(u32)(mode==47)};service(0,0,0,arg);
  u32 tile[10]={(u32)(s32)rectangle.x,(u32)(s32)rectangle.y,(u32)(s32)rectangle.w,(u32)(s32)rectangle.h,204,204,204};emit(1,tile);
  u32 track[10]={mode==46?90:216,mode==46?337:87,mode==46?112:4,mode==46?8:224,77,77,77};emit(1,track);
  u32 outline[10]={mode==46?89:215,mode==46?335:85,mode==46?114:6,mode==46?12:228,136,136,136};emit(2,outline);
 }else if(mode>=48&&mode<=50){u32 gradient[10]={149,355+18*(u32)(mode-48),65,13,0,0,0,0,0,0};gradient[7+mode-48]=255;emit(3,gradient);}
}
int main(void){
 assert(sizeof(void*)==8&&(uintptr_t)world.arena>UINT32_MAX);
 for(scenario=0;scenario<131072;scenario++){
  memset(&world,0x6A,sizeof(world));world.scratch.order=world.orders[0];world.scratch.cursor=(u8*)world.arena+16;
  rectangle.x=half(scenario);rectangle.y=half(scenario*73U);rectangle.w=half(scenario*13U);rectangle.h=half(scenario*31U);
  s32 mode;if(scenario<65536)mode=half(scenario);else if(scenario&1)mode=46+(scenario%5);else {u32 bits=next();memcpy(&mode,&bits,4);}
  expected=world;count=seen=0;reference_run=1;reference(mode);reference_run=0;garage_draw_slider_track(mode);
  assert(seen==count&&memcmp(&world,&expected,sizeof(world))==0);
 }
 puts("131072 cases; full service snapshots, fresh cursor publication and captured ordering slot");return 0;
}
'''
  with TemporaryDirectory() as d:
   c=Path(d)/'test.c';exe=Path(d)/'test';c.write_text(source)
   subprocess.run(['cc','-std=c11','-O2','-Wall','-Wextra','-Werror','-Wno-error=deprecated-non-prototype','-fsanitize=undefined','-fno-sanitize-recover=all','-I'+str(root/'include'),str(c),'-o',str(exe)],check=True)
   subprocess.run([str(exe)],check=True)
if __name__=='__main__':unittest.main()
