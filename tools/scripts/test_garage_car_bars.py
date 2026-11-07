from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess,re
body=Path('src/overlays/PAL/garage/func_80116C34.c').read_text();body=re.sub(r'__asm__\("(?:s3|a1)"\)','',body);body=re.sub(r'^extern .*;\n','',body,flags=re.M)
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
#include "rr/overlay662_car_bars.h"
#include "rr/badge_rows.h"
#include "rr/sprite_emit.h"
#include "rr/menu_choice_types.h"
static struct World {u32 guard[4];RrGpuScratchPointers scratch;RrIconSpriteContext contexts[3];RrIconSpriteContext *context;s16 status[20];RrMenuChoice4 choice;RrUiUvRect icon;u32 arena[256];u32 tail[4];} world,expected;
static struct Event {unsigned kind;u32 args[8];void *cursor;u32 *head;struct World before;} events[24];
static unsigned count,seen,scenario,profile,serial;static int reference_run;
#define CurrentDrawBuf world.context
#define SelectionRows world.choice
#define garage_D_80127300 world.status
#define garage_D_80126214 world.icon
#define RR_GPU_SCRATCH (&world.scratch)
'''+body+r'''
static u32 seed=0x16c34;static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static s16 half(u32 bits){u16 u=(u16)bits;s16 r;memcpy(&r,&u,2);return r;}
static s32 word(u32 bits){s32 r;memcpy(&r,&bits,4);return r;}
static void effect(struct World *w,unsigned kind,const u32 *args,unsigned order){
 w->context=&world.contexts[(scenario+order)%3];w->scratch.cursor=(u8*)world.arena+768;w->scratch.order=(u32*)&world.contexts[(order+1)%3];w->arena[230]^=0x12340000U|order;
 w->status[(order+7)%20]=half(scenario*73U+order);w->choice.bytes[2]=(u8)(scenario+order*17U);w->icon.u^=(u8)order;w->icon.w^=7;
 if(kind==2&&args[0]==57){w->status[w->choice.bytes[2]%20]=(profile&1)?2:1;}
}
static void *service(unsigned kind,u32 *head,void *cursor,const u32 *args){struct World *w=reference_run?&expected:&world;unsigned order=serial++;
 assert((uintptr_t)head>UINT32_MAX&&(uintptr_t)cursor>UINT32_MAX);
 if(reference_run){assert(count<24);events[count].kind=kind;events[count].cursor=cursor;events[count].head=head;memcpy(events[count].args,args,32);events[count++].before=*w;}
 else{assert(seen<count);struct Event *e=&events[seen++];assert(e->kind==kind&&e->head==head&&e->cursor==cursor&&memcmp(e->args,args,32)==0&&memcmp(w,&e->before,sizeof(*w))==0);}
 effect(w,kind,args,order);return (u8*)cursor+(kind==0?20:kind==1?16:12);
}
void *AddSpritePrim(u32 *h,void *p,s32 x,s32 y,RrUiUvRect *r,u16 clut,s32 flags){u32 a[8]={(u32)x,(u32)y,r->u,r->v,r->w,r->h,clut,(u32)flags};return service(0,h,p,a);}
void *AddTilePrim(u32 *h,void *p,s32 x,s32 y,s32 w,s32 hgt,s32 r,s32 g,s32 b){u32 a[8]={(u32)x,(u32)y,(u32)w,(u32)hgt,(u32)r,(u32)g,(u32)b};return service(1,h,p,a);}
void *AddDrawModePrim(u32 *h,void *p,u16 page){u32 a[8]={page};return service(2,h,p,a);}
static void reference(s32 selected){
 void *cursor=expected.scratch.cursor;u32 strip[8]={40,288,0,64,240,24,0x7FA4,1};cursor=service(0,&expected.context->head70,cursor,strip);
 for(unsigned i=0;i<20;i++)if(expected.status[i]!=0){u32 a[8]={41+12*i,288,10,22,255,189,0};if((s32)i==selected){a[4]=240;a[5]=122;a[6]=20;}cursor=service(1,&expected.context->head70,cursor,a);}
 u32 page[8]={57};cursor=service(2,&expected.context->head70,cursor,page);expected.scratch.cursor=cursor;
 if(expected.status[expected.choice.bytes[2]%20]==2){
  RrUiUvRect icon=expected.icon;u32 a[8]={142,256,icon.u,icon.v,icon.w,icon.h,0x7CBB,0};cursor=service(0,&expected.context->head70,cursor,a);
  u32 last[8]={56};expected.scratch.cursor=service(2,&expected.context->head70,cursor,last);
 }
}
int main(void){assert(sizeof(void*)==8);
 for(scenario=0;scenario<65536;scenario++)for(profile=0;profile<4;profile++){
  memset(&world,0xA6,sizeof(world));world.context=&world.contexts[0];world.scratch.cursor=(u8*)world.arena+16;world.scratch.order=(u32*)&world.contexts[0];world.choice.bytes[2]=(u8)scenario;
  for(unsigned i=0;i<20;i++)world.status[i]=(profile&2)&&!(scenario&(1U<<(i%16)))?0:half(scenario*(i*2+1)+i);
  world.icon=(RrUiUvRect){(u8)scenario,(u8)(scenario>>8),31,17};s32 selected=profile&2?word(next()):(s32)(scenario%24)-2;
  expected=world;count=seen=serial=0;reference_run=1;reference(selected);reference_run=0;serial=0;garage_func_80116C34(selected);
  assert(count==seen&&memcmp(&world,&expected,sizeof(world))==0);
 }
 puts("262144 cases;fresh status/context/cursor state,all16-bit status encodings and controller bytes");return 0;
}
'''
with TemporaryDirectory() as d:
 c=Path(d)/'test.c';exe=Path(d)/'test';c.write_text(source)
 subprocess.run(['cc','-std=gnu11','-O2','-Wall','-Wextra','-Werror','-Wno-error=deprecated-non-prototype','-fsanitize=undefined','-fno-sanitize-recover=all','-Iinclude',str(c),'-o',str(exe)],check=True)
 subprocess.run([str(exe)],check=True)
