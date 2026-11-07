"""Two menu constructors, actual work-buffer/reset bodies and native arena owners."""
from overlay_units import read_c_source
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

class OverlayMenuInitializersTest(unittest.TestCase):
    def test_resource_publication_and_ordered_layout_boundaries(self):
        root=Path(__file__).resolve().parents[2]
        clear=re.sub(r'^extern[^\n]*;\n','',read_c_source(root/'tools/scripts/fixtures/legacy/func_8002A604.c'),flags=re.M)
        reset=re.sub(r'^extern[^\n]*;\n','',read_c_source(root/'tools/scripts/fixtures/legacy/func_8003A198.c'),flags=re.M)
        bodies='\n'.join(read_c_source(root/f'src/overlays/PAL/{idx}/func_{addr}.c') for idx,addr in [(663,'801148D8'),(665,'80114FD8')])
        source=r'''
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4 || sizeof(t)==n,"target size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4 || offsetof(t,f)==n,"target offset")
#include "rr/overlay_menu_init.h"
typedef union {u32 words[8];u8 bytes[32];} Work32;
typedef union {u32 words[3];u8 bytes[12];} Work12;
typedef struct {s32 kind,args[10];void *data;u8 *base,*second;void *published;RrUiRow24 *ui;s32 frame,counter,c4,c8,condition;s16 selection,limit,half20;s8 flag1,flag2;} Event;
static struct World {u32 guard[4];s32 frame,counter,c4,c8,condition;s16 selection,limit;s8 flag1,flag2;RrMenuArenas arenas;u8 *base;void *published;RrUiRow24 *ui;RrUiRow24 rows[3][8];Work32 work32;Work12 work12;s32 driver[3];u32 calls;Event log[11];u32 tail[4];} world,expected;
/* The real resource owner must contain the2D000 offset; it is never mapped at
 * a synthetic PAL address. Constructors must not modify this backing memory. */
static u8 resources[3][0x2D100],original_resources[3][0x2D100];
#define ModeFrame world.frame
#define ScreenFrame world.counter
#define MenuCursor world.selection
#define MenuItemCount world.limit
#define LoadState world.flag1
#define MenuScriptStep world.flag2
#define ovl663_extra_courses_enabled world.c4
#define ovl663_record_kind world.c8
#define ovl663_available_course_count world.condition
#define MenuArenas world.arenas
#define CdDirectoryBuffer world.base
#define LoadCursor world.published
#define UiRows world.ui
#define MenuFrameState world.work32.words[0]
#define ScreenFadeState world.work12.words[0]
void PsyqBzero(void *data,s32 size);
''' + clear + reset + bodies + r'''
static u32 seed=0x148d8,scenario;static unsigned variant;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static s32 word(u32 raw){s32 value;memcpy(&value,&raw,4);return value;}
static s16 half(u32 raw){u16 bits=raw;s16 value;memcpy(&value,&bits,2);return value;}
static unsigned ui_id(struct World *v){unsigned i;for(i=0;i<3;i++)if(v->ui==world.rows[i])return i;assert(0);return 0;}
static void effect(struct World *v,s32 kind,const s32 a[10]){
 unsigned step=v->calls;
 if(kind<2)return;
 if(kind==2){
  v->arenas.first=resources[(scenario+2)%3]+0x1000;
  v->arenas.second=resources[(scenario+1)%3]+0x180+(scenario&63)*4;
  v->base=resources[(scenario+2)%3];v->published=resources[scenario%3]+0x710;
  return;
 }
 v->ui=world.rows[(scenario+step)%3];
 v->frame=word(scenario*0x9e3779b9U+step);v->counter=word(scenario^0xa1234567U^step);
 v->c4=word(scenario^0xb1234567U^step);v->c8=word(scenario^0xc1234567U^step);
 v->condition=(scenario&1)?word(0x80000001U+(step<<4)):0;
 v->selection=half(scenario+step);v->limit=half(scenario*73U+step);v->flag1=(s8)(scenario+step);v->flag2=(s8)(scenario+step+9);
 v->base=resources[(scenario+step)%3];v->arenas.second=resources[(scenario+step+1)%3]+0x100+step;
 v->published=resources[(scenario+step+2)%3]+0x200+step;
 if(kind==3){RrUiRow24 *r=&v->rows[(scenario+step)%3][a[0]];r->half04=half((u32)a[2]);r->half06=half((u32)a[3]);r->half08=half((u32)a[4]);r->byte01=(u8)a[9];}
 if(kind==4)v->driver[0]=a[0];
 if(kind==5){v->driver[0]=a[0];v->driver[1]=a[1];v->driver[2]=a[2];}
}
static void observe(struct World *v,s32 kind,const s32 a[10],void *data){
 unsigned i;Event *e;assert(v->calls<11);e=&v->log[v->calls++];
 e->kind=kind;for(i=0;i<10;i++)e->args[i]=a[i];e->data=data;e->base=v->base;e->second=v->arenas.second;e->published=v->published;e->ui=v->ui;
 e->frame=v->frame;e->counter=v->counter;e->c4=v->c4;e->c8=v->c8;e->condition=v->condition;e->selection=v->selection;e->limit=v->limit;
 e->flag1=v->flag1;e->flag2=v->flag2;e->half20=v->rows[ui_id(v)][0].half20;effect(v,kind,a);
}
void PsyqBzero(void *data,s32 size){
 unsigned step=world.calls;s32 a[10]={size};
 assert(step<2 && data==(step?(void*)world.work12.words:(void*)world.work32.words) && size==(step?12:32));
 memset(data,0,size);observe(&world,step,a,data);
}
void InitMenuArenas(void){
 s32 a[10]={0};assert(variant==0 && world.calls==2 && world.c4==0 && world.c8==0 && world.counter==0 && world.flag1==0 && world.flag2==0);
 assert(world.arenas.second==world.base+0x2D000);
 observe(&world,2,a,0);
}
void InitUiRow(s32 a,s32 b,s32 c,s32 d,s32 e,s32 f,s32 g,s32 h,s32 i,s32 j,...){
 s32 args[10]={a,b,c,d,e,f,g,h,i,j};
 assert(world.calls>=(variant?2U:3U) && a==(s32)world.calls-(variant?2:3));
 if(!variant && world.calls==3)assert(world.published==world.arenas.second);
 observe(&world,3,args,0);
}
void SetupRenderer(s32 shift){s32 a[10]={shift};assert(variant==0 && world.calls==7 && shift==5);observe(&world,4,a,0);}
void SetMenuFrameOptions(s32 a,s32 b,s32 c){s32 args[10]={a,b,c};assert(variant==1 && world.calls==10 && a==1 && b==1 && c==1);observe(&world,5,args,0);}
static const s32 layout663[4][10]={{0,118,152,68,32,0,0,1,0,122},{1,118,192,68,32,0,0,1,1,16},{2,118,232,68,32,0,0,1,1,17},{3,118,272,68,32,0,0,1,0,85}};
static const s32 layout665[8][10]={{0,12,72,68,32,0,0,1,0,86},{1,12,112,68,32,0,0,1,0,87},{2,12,152,68,32,0,0,1,0,88},{3,12,192,68,32,0,0,1,0,85},{4,100,72,98,32,0,1,1,10,89},{5,100,112,98,32,0,1,1,11,89},{6,100,152,68,32,0,1,1,0,91},{7,100,192,68,32,0,1,1,0,92}};
static void reference(void){
 unsigned i;s32 a[10]={32};
 if(!variant){expected.frame=0;expected.counter=0;expected.selection=0;expected.limit=3;}
 memset(&expected.work32,0,32);observe(&expected,0,a,world.work32.words);
 a[0]=12;memset(&expected.work12,0,12);observe(&expected,1,a,world.work12.words);
 if(!variant){
  expected.c4=0;expected.c8=0;expected.counter=0;expected.flag1=0;expected.flag2=0;
  expected.arenas.second=expected.base+0x2D000;
  a[0]=0;observe(&expected,2,a,0);expected.published=expected.arenas.second;
  for(i=0;i<4;i++)observe(&expected,3,layout663[i],0);
  if(expected.condition==0)expected.rows[ui_id(&expected)][0].half20=1;
  a[0]=5;observe(&expected,4,a,0);
 }else{
  for(i=0;i<8;i++)observe(&expected,3,layout665[i],0);
  a[0]=1;a[1]=1;a[2]=1;observe(&expected,5,a,0);
 }
}
int main(void){unsigned k,i,v;
 for(i=0;i<sizeof(resources);i++)((u8*)resources)[i]=(u8)next();memcpy(original_resources,resources,sizeof(resources));
 for(v=0;v<2;v++)for(k=0;k<65536;k++){
  for(i=0;i<sizeof(world);i++)((u8*)&world)[i]=(u8)next();
  variant=v;scenario=k;world.calls=0;world.base=resources[k%3];world.arenas.first=resources[(k+1)%3]+64;world.arenas.second=resources[(k+2)%3]+96;
  world.published=resources[(k+1)%3]+128;world.ui=world.rows[(k+2)%3];
  for(i=0;i<3;i++)world.rows[i][0].half20=half(k|0x1000U);
  assert((uintptr_t)world.base>UINT32_MAX && (uintptr_t)world.arenas.second>UINT32_MAX && (uintptr_t)world.ui>UINT32_MAX);
  memcpy(&expected,&world,sizeof(world));reference();
  if(v)ovl665_func_80114FD8();else ovl663_init_menu();
  assert(memcmp(&world,&expected,sizeof(world))==0);
 }
 assert(memcmp(resources,original_resources,sizeof(resources))==0);
 return 0;
}
'''
        with TemporaryDirectory() as temp:
            p=Path(temp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
