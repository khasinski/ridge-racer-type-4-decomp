"""Vertical thumb dragging: signed narrowing, capture gates and fresh services."""
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess,unittest
class ScrollDragTest(unittest.TestCase):
 def test_geometry_and_service_state(self):
  root=Path(__file__).resolve().parents[2]
  body=(root/'src/overlays/PAL/garage/func_80123F90.c').read_text()
  code=r'''
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
#include "rr/overlay662_scroll_drag.h"
static struct World {u32 guard[4];RrGarageSelectionPrefix g;RrGarageWorkingStorage w;s32 metrics[4];u16 grab,pad;u32 tail[4];} world,expected;
static struct Event {unsigned id; s32 args[4];struct World before;} events[2];
static unsigned count,seen,n,profile;static int reference_run;
#define garage_D_801276A8 world.g
#define garage_D_80127668 world.w
#define garage_D_80127280 world.metrics[0]
#define garage_D_80127284 world.metrics[1]
#define garage_D_80127288 world.metrics[2]
#define garage_D_8012728C world.metrics[3]
#define garage_D_8012727E world.grab
#define garage_D_8012767E world.w.points.cursor14.y
'''+body+r'''
static u32 seed=0x23f90;static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static s16 half(u32 bits){u16 u=(u16)bits;s16 r;memcpy(&r,&u,2);return r;}
static void effect(struct World *w,unsigned id,int release){
 w->g.phaseCC+=17+id;w->w.words[15]^=0xABC00000U|id;
 w->w.points.cursor14.y=half(n*73U+id*31U);w->g.reset08[1]=half(n*13U+id*17U);
 if(id==0){w->w.words[1]=(w->w.words[1]&~1U)|((profile>>1)&1);w->w.words[0]^=0x40000000U;w->g.config1C=(u8)(n%256);}
 else{w->g.config1C=(u8)(4+n%252);w->grab=(u16)(n+19);for(unsigned i=0;i<4;i++)w->metrics[i]=(s32)(n+i+101);if(release)w->w.words[0]^=0x10000000U;}
}
static void service(unsigned id,s32 x,s32 y,s32 width,s32 height){s32 args[4]={x,y,width,height};struct World *w=reference_run?&expected:&world;
 if(reference_run){assert(count<2);events[count].id=id;memcpy(events[count].args,args,sizeof(args));events[count++].before=*w;}
 else{assert(seen<count);struct Event *e=&events[seen++];assert(e->id==id&&memcmp(e->args,args,sizeof(args))==0&&memcmp(w,&e->before,sizeof(*w))==0);}
 effect(w,id,x==13);
}
s32 garage_func_8011E6C8(s32 x,s32 y,s32 width,s32 height){service(0,x,y,width,height);return (profile>>2)&1;}
void garage_func_8011E7C0(s32 x,s32 y,s32 width,s32 height){service(1,x,y,width,height);}
static void reference(void){
 int active=(expected.w.words[0]&8)!=0;
 if(active){if(expected.w.words[1]&4){expected.w.words[0]&=~8U;service(1,13,63,294,346);return;}}
 else{
  if(expected.g.config1C<4)return;service(0,216,87,4,224);
  if(((profile>>2)&1)||!(expected.w.words[1]&1))return;
  expected.w.words[4]=47;expected.w.words[0]|=8;service(1,216,87,4,224);
  int64_t divisor=expected.g.config1C/2,thumb=64/divisor,track=224/divisor,range=64-thumb,travel=224-track;
  int64_t projected=((int64_t)expected.w.points.cursor14.y-87)*range/travel;
  expected.metrics[0]=(s32)thumb;expected.metrics[1]=(s32)range;expected.metrics[2]=(s32)track;expected.metrics[3]=(s32)travel;
  int64_t offset=projected-expected.g.reset08[1];
  if(offset<0||offset>=thumb)offset=(track/2)*range/travel;expected.grab=(u16)offset;
 }
 int64_t projected=((int64_t)expected.w.points.cursor14.y-87)*expected.metrics[1]/expected.metrics[3];
 s16 narrowed=half((u32)(projected-expected.grab));
 if(narrowed<0)narrowed=0;else if(narrowed>expected.metrics[1])narrowed=half((u32)expected.metrics[1]);
 expected.g.reset08[1]=narrowed;
}
int main(void){assert(sizeof(void*)==8);
 for(n=0;n<65536;n++)for(profile=0;profile<16;profile++){
  memset(&world,0xA6,sizeof(world));world.w.words[0]=next();world.w.words[1]=next();
  world.w.words[0]=(world.w.words[0]&~8U)|(profile&1?8:0);world.w.words[1]=(world.w.words[1]&~4U)|(profile&8?4:0);
  world.g.config1C=(u8)n;world.g.reset08[1]=half(n*31U);world.w.points.cursor14.y=half(n);world.grab=(u16)n;
  unsigned divisor=2+n%126;world.metrics[0]=(s32)(64/divisor);world.metrics[1]=64-world.metrics[0];world.metrics[2]=(s32)(224/divisor);world.metrics[3]=224-world.metrics[2];
  expected=world;count=seen=0;reference_run=1;reference();reference_run=0;garage_func_80123F90();assert(seen==count&&memcmp(&world,&expected,sizeof(world))==0);
 }
 puts("1048576 guarded cases;all16-bit cursor/grab values,all zoom encodings and service snapshots");return 0;
}
'''
  with TemporaryDirectory() as d:
   c=Path(d)/'test.c';exe=Path(d)/'test';c.write_text(code)
   subprocess.run(['cc','-std=gnu11','-O2','-Wall','-Wextra','-Werror','-Wno-error=deprecated-non-prototype','-fsanitize=undefined','-fno-sanitize-recover=all','-I'+str(root/'include'),str(c),'-o',str(exe)],check=True)
   subprocess.run([str(exe)],check=True)
if __name__=='__main__':unittest.main()
