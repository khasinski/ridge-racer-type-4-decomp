"""Scale-panel argument capture and full cursor publication; renderer services simulated."""
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess
import unittest

class Overlay662ScalePanelTest(unittest.TestCase):
    def test_scale_coordinates_and_mutating_services(self):
        root=Path(__file__).resolve().parents[2]
        body=(root/'src/overlays/PAL/garage/func_8011C610.c').read_text()
        source=r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4||sizeof(t)==n,"PAL size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4||offsetof(t,f)==n,"PAL offset")
#include "rr/overlay662_scale_panel.h"
typedef struct {u32 guard[4];RrGarageSelectionPrefix garage;RrGpuScratchPointers scratch;u32 tail[4];} State;
typedef struct {unsigned kind,cursor,bank,head;u32 args[10],hash;} Event;
typedef struct {State s;Event events[6];unsigned calls;} World;
static World actual,reference,initial,*w;
static u32 heads[2][704],headSnapshot[2][704],arena[256];
static unsigned scenario,mutation;
#undef RR_GARAGE_SCRATCH
#define RR_GARAGE_SCRATCH (&w->s.scratch)
#define garage_logo_state w->s.garage
static u8 *service(unsigned kind,u32 *order,u8 *cursor,const u32 *args,unsigned n){unsigned i;u32 hash=2166136261U;Event *e;
 assert(w->calls<6);assert(order>=heads[0]&&order<heads[0]+704);
 assert(cursor>=(u8*)arena&&cursor<(u8*)(arena+256));assert((uintptr_t)cursor>UINT32_MAX);
 for(i=0;i<sizeof(w->s);i++)hash=(hash^((u8*)&w->s)[i])*16777619U;
 e=w->events+w->calls++;memset(e,0,sizeof(*e));e->kind=kind;e->cursor=cursor-(u8*)arena;e->head=order-heads[0];e->hash=hash;memcpy(e->args,args,n*4);
 if(mutation){w->s.scratch.order=heads[1]+scenario%128;w->s.scratch.cursor=(u8*)arena+768;w->s.garage.config1C=0;w->s.garage.reset08[0]=(s16)~scenario;w->s.garage.reset08[1]=(s16)(scenario*13);w->s.garage.shadeD0=(u8)~scenario;}
 return cursor+(4*(w->calls+1));
}
RrGpuLineF4Packet *garage_add_rect_outline(u32 *o,RrGpuLineF4Packet *p,u32 x,u32 y,u32 width,u32 height,u32 r,u32 g,u32 b){u32 args[]={x,y,width,height,r,g,b};return (RrGpuLineF4Packet*)service(1,o,(u8*)p,args,7);}
u8 *garage_add_draw_area(u32 *o,u8 *p,u32 x,u32 y,u32 width,u32 height){u32 args[]={x,y,width,height};return service(2,o,p,args,4);}
RrGpuPolyFT4Packet *garage_add_textured_rect(u32 *o,RrGpuPolyFT4Packet *p,u32 x,u32 y,u32 width,u32 height,u32 u,u32 v,u32 uw,u32 vh,u32 clut,u32 page){u32 args[]={x,y,width,height,u,v,uw,vh,clut,page};return (RrGpuPolyFT4Packet*)service(3,o,(u8*)p,args,10);}
''' + body + r'''
static s16 narrow(unsigned bits){u16 u=bits;s16 v;memcpy(&v,&u,2);return v;}
static void expected(void){State *s=&w->s;u32 *order=s->scratch.order+136;u32 width=64/(s->garage.config1C/2),height=128/(s->garage.config1C/2);
 u32 args[6][10]={{(u32)(s32)narrow((u16)s->garage.reset08[0]+237),(u32)(s32)narrow((int)s->garage.reset08[1]*2+71),width,height,0,s->garage.shadeD0,0},
 {12,61,296,352},{236,69,66,132,71,1,66,66,4272,12},{236,69,66,132,3,1,66,66,4273,12},{237,71,64,128},{236,69,66,132,255,255,255}};
 static const unsigned kinds[]={1,2,3,3,2,1},counts[]={7,4,10,10,4,7};unsigned i;
 for(i=0;i<6;i++)s->scratch.cursor=service(kinds[i],order,s->scratch.cursor,args[i],counts[i]);
}
int main(void){unsigned i;
 memset(heads,0x75,sizeof(heads));memcpy(headSnapshot,heads,sizeof(heads));memset(arena,0x93,sizeof(arena));
 for(scenario=0;scenario<65536;scenario++)for(mutation=0;mutation<2;mutation++){
  memset(&initial,0,sizeof(initial));initial.s.garage.config1C=2+scenario%254;
  initial.s.garage.reset08[0]=narrow(scenario);initial.s.garage.reset08[1]=narrow(scenario*17);initial.s.garage.shadeD0=scenario;
  for(i=0;i<4;i++){initial.s.guard[i]=scenario+i;initial.s.tail[i]=~(scenario+i);}
  initial.s.scratch.cursor=(u8*)arena+4*(scenario%64);initial.s.scratch.order=heads[0]+scenario%128;
  actual=reference=initial;w=&actual;{void (*render)(s32)=garage_draw_preview_window;render(0);}w=&reference;expected();assert(!memcmp(&actual,&reference,sizeof(actual)));assert(actual.calls==6);
 }
 assert(!memcmp(heads,headSnapshot,sizeof(heads)));for(i=0;i<sizeof(arena);i++)assert(((u8*)arena)[i]==0x93);return 0;
}
'''
        with TemporaryDirectory() as temp:
            p=Path(temp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
