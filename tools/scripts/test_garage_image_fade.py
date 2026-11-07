from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess,re
root=Path(__file__).resolve().parents[2];body=(root/'src/overlays/PAL/garage/func_80119898.c').read_text();body=re.sub(r'__asm__\("(?:v0|v1)"\)','',body)
body=body.replace('extern s32 ScreenFrame;','').replace('extern s16 garage_image_fade_phase;','')
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
#include "rr/garage_image_fade.h"
static struct World {u32 guard[4];RrGpuScratchPointers scratch;RrIconSpriteContext contexts[3];RrIconSpriteContext *context;s16 phase;u16 pad;s32 clock;u32 arena[256];u32 tail[4];} world,expected;
static struct Event {unsigned id;u32 args[12];u8 *cursor;u32 *head;struct World before;} events[5];
static unsigned count,seen,scenario;static int ref;static s32 sineReturns[2];static unsigned sineIndex;static s32 geometry[4][4];static u16 expectedShade;
#define CurrentDrawBuf world.context
#define ScreenFrame world.clock
#define garage_image_fade_phase world.phase
#define RR_GPU_SCRATCH (&world.scratch)
'''+body+r'''
static s32 bits32(u32 b){s32 r;memcpy(&r,&b,4);return r;}
static s16 bits16(u32 b){u16 v=(u16)b;s16 r;memcpy(&r,&v,2);return r;}
static void effect(struct World *w,unsigned id){
 w->context=&world.contexts[(scenario+id)%3];w->phase=bits16((u32)scenario*73U+id*17U);w->clock=bits32((u32)w->clock+0x1234567U);
 w->scratch.cursor=(u8*)world.arena+(id==1?64:768);w->scratch.order=(u32*)&world.contexts[(id+1)%3];w->arena[231]^=0xABC00000U|id;
}
static void service(unsigned id,const u32 *args,u8 *cursor,u32 *head){struct World *w=ref?&expected:&world;
 if(ref){assert(count<5);events[count].id=id;memcpy(events[count].args,args,48);events[count].cursor=cursor;events[count].head=head;events[count++].before=*w;}
 else{assert(seen<count);struct Event *e=&events[seen++];assert(e->id==id&&e->cursor==cursor&&e->head==head&&memcmp(e->args,args,48)==0&&memcmp(w,&e->before,sizeof(*w))==0);}
 effect(w,id);
}
s32 rsin(s32 angle){u32 a[12]={(u32)angle};service(1,a,0,0);assert(sineIndex<2);return sineReturns[sineIndex++];}
struct S32 *EmitPolyFT3(u32 *head,struct S32 *packet,void *x,void *y,void *u,void *v,u16 tp,u16 clut,u16 mode,u16 shade){
 s32 *arrays[4]={x,y,u,v};u32 a[12];for(unsigned i=0;i<4;i++)for(unsigned j=0;j<3;j++)a[3*i+j]=(u32)arrays[i][j];
 /* Constant material arguments are separately checked;event payload holds all coordinates. */
 assert(tp==42&&clut==0x1028&&mode==0&&shade==expectedShade);service(2,a,(u8*)packet,head);
 for(unsigned i=0;i<4;i++)for(unsigned j=0;j<3;j++)arrays[i][j]=bits32((u32)arrays[i][j]+0x80000001U+i*17U+j);
 return (struct S32*)((u8*)packet+32);
}
void *AddTilePrim(u32 *head,void *packet,s32 x,s32 y,s32 w,s32 h,s32 r,s32 g,s32 b){u32 a[12]={(u32)x,(u32)y,(u32)w,(u32)h,(u32)r,(u32)g,(u32)b};service(3,a,packet,head);return (u8*)packet+16;}
static void reference(s32 mode){
 s32 amount=expected.phase;if(amount>128)amount=128;expectedShade=(u16)amount;
 for(unsigned i=0;i<4;i++){geometry[0][i]=128+64*(i&1);geometry[1][i]=i<2?128:256;geometry[2][i]=63*(i&1);geometry[3][i]=i<2?0:63;}
 unsigned si=0;
 if(mode==1){u32 a[12]={(u32)(amount*8)};service(1,a,0,0);s32 d=sineReturns[si++]/64;geometry[1][0]+=d;geometry[1][1]+=d;geometry[1][2]-=d;geometry[1][3]-=d;amount=0;}
 u32 a[12]={(u32)expected.clock*64U};service(1,a,0,0);s32 shade=64-sineReturns[si++]/64;u8 *cursor=expected.scratch.cursor;
 if(amount>0){for(unsigned tri=0;tri<2;tri++){u32 t[12];for(unsigned i=0;i<4;i++)for(unsigned j=0;j<3;j++)t[3*i+j]=(u32)geometry[i][j+tri];service(2,t,cursor,&expected.context->head70);cursor+=32;for(unsigned i=0;i<4;i++)for(unsigned j=0;j<3;j++)geometry[i][j+tri]=bits32((u32)geometry[i][j+tri]+0x80000001U+i*17U+j);}}
 else shade=0;
 u32 back[12]={(u32)geometry[0][0],(u32)geometry[1][0],(u32)geometry[0][1]-(u32)geometry[0][0],(u32)geometry[1][2]-(u32)geometry[1][0],(u32)shade,(u32)shade,(u32)shade};service(3,back,cursor,&expected.context->head70);cursor+=16;
 s16 current=expected.phase;u16 old=(u16)current;expected.scratch.cursor=cursor;
 if(current>=128)expected.phase=128;else expected.phase=bits16(old+(mode==1?8U:4U));
}
int main(void){assert(sizeof(void*)==8&&(uintptr_t)world.arena>UINT32_MAX);
 const s32 returns[]={INT32_MIN,-4096,-65,-64,-63,-1,0,1,63,64,65,4096,INT32_MAX};
 for(scenario=0;scenario<196608;scenario++){
  memset(&world,0xA5,sizeof(world));world.context=&world.contexts[0];world.phase=bits16(scenario);world.clock=bits32(scenario*0x8642ABCDU);world.scratch.cursor=(u8*)world.arena+16;world.scratch.order=(u32*)&world.contexts[0];
  s32 mode=scenario/65536==0?0:scenario/65536==1?1:-7;sineReturns[0]=returns[scenario%13];sineReturns[1]=returns[(scenario/13)%13];
  expected=world;count=seen=sineIndex=0;ref=1;reference(mode);ref=0;sineIndex=0;garage_draw_color_preview_image(mode);assert(count==seen&&memcmp(&world,&expected,sizeof(world))==0);
 }
 puts("196608 cases:all phase encodings in three modes,full32 sine stress,full native pointers and service snapshots");return 0;
}
'''
import unittest
class FadeImageTest(unittest.TestCase):
 def test_geometry_and_callbacks(self):
  with TemporaryDirectory() as d:
   c=Path(d)/'test.c';exe=Path(d)/'test';c.write_text(source)
   subprocess.run(['cc','-std=gnu11','-O2','-Wall','-Wextra','-Werror','-Wno-error=deprecated-non-prototype','-fsanitize=undefined','-fno-sanitize-recover=all','-I'+str(root/'include'),str(c),'-o',str(exe)],check=True)
   subprocess.run([str(exe)],check=True)
if __name__=="__main__":unittest.main()
