"""Preview dragging: paired grabs, signed halfword wrap and two hit services."""
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess,unittest
class PreviewDragTest(unittest.TestCase):
 def test_grab_and_coordinate_oracle(self):
  root=Path(__file__).resolve().parents[2]
  body=(root/'src/overlays/PAL/garage/func_801246E4.c').read_text()
  source=r'''
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4||sizeof(t)==n,"PAL size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4||offsetof(t,f)==n,"PAL offset")
#include "rr/garage_preview_drag.h"
static struct World {u32 guard[4];RrGarageSelectionPrefix g;RrGarageWorkingStorage w;RrGaragePreviewGrab grab;s32 thumb,travel;u32 tail[4];} world,expected;
static struct Event {unsigned id;s32 args[4];struct World before;} events[3];
static unsigned count,seen,n,profile,hits;static int reference_run;
#define garage_logo_state world.g
#define garage_logo_work world.w
#define garage_preview_drag_grab world.grab
#define garage_preview_drag_thumb world.thumb
#define garage_preview_drag_travel world.travel
'''+body+r'''
static u32 seed=0x246e4;static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static s16 half(u32 b){u16 u=(u16)b;s16 r;memcpy(&r,&u,2);return r;}
static void effect(struct World *w,unsigned id){
 w->w.points.cursor14.x=half(n*73U+id*17U);w->w.points.cursor14.y=half(n*31U+id*29U);
 w->g.reset08[0]=half(n*13U+id*3U);w->g.reset08[1]=half(n*17U+id*5U);
 w->g.phaseCC+=id+19;w->w.words[15]^=0xABCD0000U|id;
 if(id==0){w->w.words[1]=(w->w.words[1]&~1U)|((profile>>1)&1);w->g.config1C=(u8)n;}
 if(id==1){w->g.config1C=(u8)(4+n%252);w->grab[0]=123;w->grab[1]=456;w->thumb=19;w->travel=45;}
 if(id==2){w->thumb=(s32)(n%33);w->travel=32+(s32)(n%33);w->grab[0]=(u16)(n+7);w->grab[1]=(u16)(n+9);}
}
static void service(unsigned id,s32 x,s32 y,s32 width,s32 height){s32 a[4]={x,y,width,height};struct World *w=reference_run?&expected:&world;
 if(reference_run){assert(count<3);events[count].id=id;memcpy(events[count].args,a,sizeof(a));events[count++].before=*w;}
 else{assert(seen<count);struct Event *e=&events[seen++];assert(e->id==id&&memcmp(e->args,a,sizeof(a))==0&&memcmp(w,&e->before,sizeof(*w))==0);}
 effect(w,id);
}
s32 garage_cursor_outside_rect(s32 x,s32 y,s32 width,s32 height){unsigned id=hits++==0?0:2;service(id,x,y,width,height);return id==0?(profile>>2)&1:(profile>>4)&1;}
void garage_set_cursor_bounds(s32 x,s32 y,s32 width,s32 height){service(1,x,y,width,height);}
static void reference(void){
 int active=(expected.w.words[0]&8)!=0;
 if(active){if(expected.w.words[1]&4){expected.w.words[0]&=~8U;service(1,13,63,294,346);return;}}
 else{
  if(expected.g.config1C<4)return;service(0,237,71,64,128);
  if(((profile>>2)&1)||!(expected.w.words[1]&1))return;
  expected.w.words[4]=51;expected.w.words[0]|=8;service(1,237,71,64,128);
  s32 thumb=64/(expected.g.config1C/2);expected.thumb=thumb;expected.travel=64-thumb;
  service(2,half((u32)expected.g.reset08[0]+237),half((u32)expected.g.reset08[1]*2U+71),half((u32)expected.thumb),thumb*2);
  if((profile>>4)&1)expected.grab[0]=expected.grab[1]=(u16)(expected.thumb/2);
  else{
   expected.grab[0]=(u16)((int64_t)expected.w.points.cursor14.x-237-expected.g.reset08[0]);
   expected.grab[1]=(u16)((int64_t)expected.w.points.cursor14.y/2-35-(u16)expected.g.reset08[1]);
  }
 }
 s16 x=half((u32)expected.w.points.cursor14.x-expected.grab[0]-237U);
 s16 y=half((u32)(((int64_t)expected.w.points.cursor14.y-71)/2-expected.grab[1]));
 if(x<0)x=0;else if(x>expected.travel)x=half((u32)expected.travel);
 if(y<0)y=0;else if(y>expected.travel)y=half((u32)expected.travel);
 expected.g.reset08[0]=x;expected.g.reset08[1]=y;
}
int main(void){assert(sizeof(void*)==8);
 for(n=0;n<65536;n++)for(profile=0;profile<32;profile++){
  memset(&world,0xA6,sizeof(world));world.w.words[0]=next();world.w.words[1]=next();
  world.w.words[0]=(world.w.words[0]&~8U)|(profile&1?8:0);world.w.words[1]=(world.w.words[1]&~4U)|(profile&8?4:0);
  world.g.config1C=(u8)n;world.w.points.cursor14.x=half(n);world.w.points.cursor14.y=half(n*73U);
  world.g.reset08[0]=half(n*13U);world.g.reset08[1]=half(n*17U);world.grab[0]=(u16)n;world.grab[1]=(u16)(n*31U);world.thumb=(s32)(n%33);world.travel=32+(s32)(n%33);
  expected=world;count=seen=hits=0;reference_run=1;reference();reference_run=0;hits=0;garage_drag_preview_viewport();assert(seen==count&&memcmp(&world,&expected,sizeof(world))==0);
 }
 puts("2097152 guarded cases;paired grabs,all16-bit coordinates and fresh hit/clip services");return 0;
}
'''
  with TemporaryDirectory() as d:
   c=Path(d)/'test.c';exe=Path(d)/'test';c.write_text(source)
   subprocess.run(['cc','-std=gnu11','-O2','-Wall','-Wextra','-Werror','-Wno-error=deprecated-non-prototype','-fsanitize=undefined','-fno-sanitize-recover=all','-I'+str(root/'include'),str(c),'-o',str(exe)],check=True)
   subprocess.run([str(exe)],check=True)
if __name__=='__main__':unittest.main()
