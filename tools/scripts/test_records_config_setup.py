"""Configuration constructor: full word mode and a genuine two-row badge owner."""
from overlay_units import read_c_source
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

class Overlay663ConfigSetupTest(unittest.TestCase):
    def test_word_mode_and_fresh_second_badge(self):
        root=Path(__file__).resolve().parents[2]
        body=read_c_source(root/'src/overlays/PAL/records/func_80115028.c')
        clear=re.sub(r'^extern[^\n]*;\n','',read_c_source(root/'tools/scripts/fixtures/legacy/func_8002A604.c'),flags=re.M)
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
typedef struct {s32 kind,args[10];void *data;RrBadgeRow1C *badge;s32 frame,counter,local;s16 selected,mapped,current;} Event;
static struct World {u32 guard[4];s32 frame,counter,local;s16 selected,mapped,current;Work32 work32;Work12 work12;RrBadgeRow1C *badge;RrBadgeRow1C badges[3][2];u32 calls;Event events[7];u32 tail[4];} world,expected;
#define ModeFrame world.frame
#define ScreenFrame world.counter
#define records_selected_record world.local
#define CourseMenuSelection world.selected
#define CourseIndex world.mapped
#define MenuCursor world.current
#define BadgeRows world.badge
#define MenuFrameState world.work32.words[0]
#define ScreenFadeState world.work12.words[0]
void PsyqBzero(void *data,s32 size);
''' + clear + body + r'''
static u32 seed=0x15028,scenario;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static s16 half(u32 raw){u16 bits=raw;s16 result;memcpy(&result,&bits,2);return result;}
static s32 word(u32 raw){s32 result;memcpy(&result,&raw,4);return result;}
static void observe(struct World *v,s32 kind,const s32 args[10],void *data){
 unsigned i;Event *event;assert(v->calls<7);event=&v->events[v->calls++];event->kind=kind;
 for(i=0;i<10;i++)event->args[i]=args[i];event->data=data;event->badge=v->badge;event->frame=v->frame;event->counter=v->counter;event->local=v->local;event->selected=v->selected;event->mapped=v->mapped;event->current=v->current;
 if(kind>=2){unsigned id=(scenario+v->calls)%3;
  v->frame=word(scenario^0xA1234567U^v->calls);v->counter=word(scenario^0xB1234567U^v->calls);v->local=(scenario&1)?2:word(0xC1234567U^v->calls);
  v->selected=half(scenario+v->calls);v->mapped=half(scenario*73U+v->calls);v->current=half(scenario*40503U+v->calls);
  v->badge=world.badges[id];v->badges[id][0].half02=half(scenario|0x1000U);v->badges[id][1].half02=half(scenario|0x2000U);
 }
}
void PsyqBzero(void *data,s32 size){
 unsigned step=world.calls;s32 args[10]={size};assert(step<2 && data==(step?(void*)world.work12.words:(void*)world.work32.words) && size==(step?12:32));
 assert(world.frame==0 && world.counter==0 && world.local==0 && world.selected==0 && world.mapped==0 && world.current==0);
 memset(data,0,size);observe(&world,step,args,data);
}
void InitUiRow(s32 a,s32 b,s32 c,s32 d,s32 e,s32 f,s32 g,s32 h,s32 i,s32 j,...){s32 args[10]={a,b,c,d,e,f,g,h,i,j};assert(world.calls>=2 && world.calls<6 && a==(s32)world.calls-2);observe(&world,2,args,0);}
void InitBadgeRow(s32 a,s32 b,s16 c,s32 d,u16 e,u16 f,u16 g){s32 args[10]={a,b,c,d,e,f,g};assert(world.calls==6);observe(&world,3,args,0);}
static const s32 rows[4][10]={{0,12,64,68,32,0,0,1,1,24},{1,12,104,68,32,0,0,1,0,85},{2,12,368,208,32,0,0,8,4,0},{3,24,88,68,32,0,0,1,0,83}};
static void reference(s32 mode){
 unsigned i;s32 args[10]={32};
 expected.frame=0;expected.counter=0;expected.local=0;expected.selected=0;expected.mapped=0;expected.current=0;
 memset(&expected.work32,0,32);observe(&expected,0,args,world.work32.words);
 args[0]=12;memset(&expected.work12,0,12);observe(&expected,1,args,world.work12.words);
 for(i=0;i<4;i++)observe(&expected,2,rows[i],0);
 {s32 badge[10]={1,49,188,222,152,0,mode==2?0xB04:0xA04};observe(&expected,3,badge,0);}
 expected.badges[(scenario+7)%3][1].half02=1;
}
int main(void){unsigned k,i;const u32 edge[]={0,1,2,3,0x10002U,0x80000002U,0xffffffffU,0x7fffffffU};
 for(k=0;k<131072;k++){
  s32 mode;
  for(i=0;i<sizeof(world);i++)((u8*)&world)[i]=(u8)next();
  scenario=k;world.calls=0;world.badge=world.badges[k%3];
  if(k<64)mode=word(edge[k&7]);else if(k&1)mode=2;else mode=word((next()&0xffff0000U)|((k>>1)&0xffffU));
  assert((uintptr_t)world.badge>UINT32_MAX);memcpy(&expected,&world,sizeof(world));reference(mode);
  records_init_selection(mode);assert(memcmp(&world,&expected,sizeof(world))==0);
 }
 return 0;
}
'''
        with TemporaryDirectory() as temp:
            p=Path(temp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
