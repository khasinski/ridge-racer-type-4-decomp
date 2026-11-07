"""Randomized menu origins, actual selection/reset helpers and typed scratch owner."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

class Overlay661ChoiceInitTest(unittest.TestCase):
    def test_random_calls_word_result_and_projection_fields(self):
        root=Path(__file__).resolve().parents[2]
        body=(root/'src/overlays/PAL/versus_select/func_80117350.c').read_text()
        fixed=(root/'src/overlays/PAL/serial_link/func_80117EF8.c').read_text()
        lookup=(root/'tools/scripts/fixtures/legacy/func_80032E9C.c').read_text()
        reset=re.sub(r'^extern[^\n]*;\n','',(root/'tools/scripts/fixtures/legacy/func_8003A198.c').read_text(),flags=re.M)
        exe=(root/'assets/PAL/main.exe').read_bytes();table=exe[0x800+0x8009F64C-0x80010000:0x800+0x8009F64C-0x80010000+8]
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
#include "rr/overlay661_init.h"
#include "rr/overlay667_init.h"
typedef struct {s32 frame,counter,bank;s16 current,limit,selected,mapped,flag;RrMenuGridPoint points[4];RrMenuArenas arenas;void *published;RrProjectionScratchPrefix scratch;RrSceneStatePrefix scene;s32 args[3];} Event;
static struct World {u32 guard[4];s32 frame,counter,bank;s16 current,limit,selected,mapped,flag,modeCounter,selectionCounter;s8 flag1,flag2;RrMenuGridPoint points[4];RrMenuArenas arenas;void *published;RrProjectionScratchPrefix scratch;RrSceneStatePrefix scene;u32 calls;Event events[3];u8 arena[3][64];u32 order[16];u32 tail[4];} world,expected;
#define RR_GPU_PROJECTION_SCRATCH (&world.scratch)
#define ModeFrame world.frame
#define ScreenFrame world.counter
#define SelectionPlayer world.bank
#define MenuCursor world.current
#define MenuItemCount world.limit
#define CourseMenuSelection world.selected
#define CourseIndex world.mapped
#define ReverseCourse world.flag
#define LinkResultsExitFlag world.modeCounter
#define CourseMenuPhase world.selectionCounter
#define LoadState world.flag1
#define MenuScriptStep world.flag2
#define PlayerCourseChoices world.points
#define MenuArenas world.arenas
#define LoadCursor world.published
#define ComCars world.scene
static u8 codes[8]={''' + ','.join(map(str,table)) + r'''};
#define CourseIndexMap codes
''' + lookup + reset + body + fixed + r'''
static u32 seed=0x17350,scenario,random_value[2];
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static s16 half(u32 raw){u16 bits=raw;s16 result;memcpy(&result,&bits,2);return result;}
static s32 word(u32 raw){s32 result;memcpy(&result,&raw,4);return result;}
static void event(struct World *v,s32 a,s32 b,s32 c){
 Event *e;assert(v->calls<3);e=&v->events[v->calls++];
 e->frame=v->frame;e->counter=v->counter;e->bank=v->bank;e->current=v->current;e->limit=v->limit;e->selected=v->selected;e->mapped=v->mapped;e->flag=v->flag;
 memcpy(e->points,v->points,sizeof(e->points));e->arenas=v->arenas;e->published=v->published;e->scratch=v->scratch;e->scene=v->scene;e->args[0]=a;e->args[1]=b;e->args[2]=c;
}
static void effect(struct World *v,unsigned step){
 v->frame=word(scenario^0xA1234567U^step);v->counter=word(scenario^0xB1234567U^step);v->bank=word(scenario^0xC1234567U^step);
 v->current=half(scenario+step);v->limit=half(scenario*73U+step);v->selected=half(scenario*40503U+17U+step);
 v->flag=half(scenario+step+8);v->mapped=half(scenario+step+9);v->flag1=(s8)(step+7);v->flag2=(s8)(step+11);
 v->points[2].first=half(scenario+step+100);v->points[3].first=half(scenario+step+200);
 v->arenas.first=world.arena[(scenario+step)%3]+8;v->arenas.second=world.arena[(scenario+step+1)%3]+16;v->published=world.arena[(scenario+step+2)%3]+24;
 v->scratch.pointers.cursor=world.arena[(scenario+step)%3]+32;v->scratch.pointers.order=world.order;
}
s32 PsyqRand(void){
 unsigned step=world.calls;assert(step<2);
 if(step==0)assert(world.frame==0 && world.counter==0 && world.current==0 && world.limit==4);
 else assert(world.points[2].first==(s16)((random_value[0]&3)+10));
 event(&world,0,0,0);effect(&world,step);return word(random_value[step]);
}
void SetMenuFrameOptions(s32 a,s32 b,s32 c){
 assert(world.calls==2 && a==0 && b==1 && c==1);
 assert(world.counter==0 && world.flag1==0 && world.flag2==0 && world.bank==0 && world.published==world.arenas.second);
 assert(world.scratch.word18==512 && world.scratch.word0C==-416 && world.scratch.word08==0 && world.scratch.word10==0 && world.scratch.word1C==0 && world.scratch.word20==0);
 event(&world,a,b,c);effect(&world,2);
}
static void reference(void){
 expected.frame=0;expected.counter=0;expected.current=0;expected.limit=4;
 event(&expected,0,0,0);effect(&expected,0);expected.points[2].first=(random_value[0]&3)+10;
 event(&expected,0,0,0);effect(&expected,1);expected.points[3].first=(random_value[1]&3)+12;
 expected.flag=expected.selected>=8;expected.mapped=codes[(u16)expected.selected&7];
 expected.counter=0;expected.flag1=0;expected.flag2=0;
 expected.scratch.word18=512;expected.scratch.word0C=-416;expected.scratch.word1C=0;expected.scratch.word20=0;expected.scratch.word08=0;expected.scratch.word10=0;
 expected.scene.words20[0]=0;expected.scene.words20[1]=0;expected.scene.words20[2]=0;expected.bank=0;expected.published=expected.arenas.second;
 event(&expected,0,1,1);effect(&expected,2);
}
int main(void){unsigned k,i;const u32 edges[]={0,3,0x7fffffffU,0x80000000U,0xffffffffU,0xffff0000U,0x12345678U,0x87654321U};
 for(k=0;k<65536;k++){
  for(i=0;i<sizeof(world);i++)((u8*)&world)[i]=(u8)next();
  scenario=k;world.calls=0;world.selected=half(k);world.arenas.first=world.arena[k%3];world.arenas.second=world.arena[(k+1)%3]+8;world.published=world.arena[(k+2)%3]+16;
  world.scratch.pointers.cursor=world.arena[k%3]+24;world.scratch.pointers.order=world.order;
  random_value[0]=(k&8)?next():edges[k&7];random_value[1]=(k&16)?next():edges[(k>>3)&7];
  assert((uintptr_t)world.scratch.pointers.cursor>UINT32_MAX && (uintptr_t)world.arenas.second>UINT32_MAX);
  memcpy(&expected,&world,sizeof(world));reference();versus_select_func_80117350();assert(memcmp(&world,&expected,sizeof(world))==0);
 }
 /* The other initializer has no random/UI boundaries: execute its complete
  * resident selection/reset closure for every signed16 selector. */
 for(k=0;k<65536;k++){
  for(i=0;i<sizeof(world);i++)((u8*)&world)[i]=(u8)next();
  world.selected=half(k);world.calls=0;world.arenas.first=world.arena[k%3];world.arenas.second=world.arena[(k+1)%3]+8;world.published=world.arena[(k+2)%3]+16;
  world.scratch.pointers.cursor=world.arena[k%3]+24;world.scratch.pointers.order=world.order;
  memcpy(&expected,&world,sizeof(world));
  expected.frame=0;expected.counter=0;expected.current=0;expected.limit=3;expected.modeCounter=0;
  expected.flag=expected.selected>=8;expected.mapped=codes[(u16)expected.selected&7];expected.flag1=0;expected.flag2=0;
  expected.scratch.word18=512;expected.scratch.word0C=-416;expected.scratch.word1C=0;expected.scratch.word20=0;expected.scratch.word08=0;expected.scratch.word10=0;
  expected.scene.words20[0]=0;expected.scene.words20[1]=0;expected.scene.words20[2]=0;expected.bank=0;expected.selectionCounter=0;expected.published=expected.arenas.second;
  serial_link_func_80117EF8();assert(memcmp(&world,&expected,sizeof(world))==0);
 }
 return 0;
}
'''
        with TemporaryDirectory() as temp:
            p=Path(temp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
