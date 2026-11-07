"""Scratchpad pointer owner, captured ordering base and real draw-mode closure."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

class GarageSetupTest(unittest.TestCase):
    def test_captured_order_and_live_cursor_publication(self):
        root=Path(__file__).resolve().parents[2]
        page=(root/'tools/scripts/fixtures/views/func_800940E0.c').read_text()
        page=re.sub(r' asm\("\$\d+"\)','',page)
        window=(root/'tools/scripts/fixtures/views/func_8009424C.c').read_text()
        builder=(root/'tools/scripts/fixtures/views/func_80093C24.c').read_text()
        builder=builder.replace('void SetDrawMode(', 'void actual_builder(')
        mode=(root/'src/overlays/PAL/garage/func_8011AC94.c').read_text()
        setup=(root/'src/overlays/PAL/garage/func_8011B400.c').read_text()
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
#include "rr/garage_primitives.h"
#include "rr/animated_sprite_grid.h"
typedef union {RrGpuDrawModePacket packets[16];u32 words[48];} Owner;
typedef struct {u8 *cursor;u32 *order;void *packet;u32 args[5];} Event;
static struct World {u32 guard[4];RrGpuScratchPointers scratch;u32 orders[3][704];Owner storage[4];u32 calls;Event log[3];u32 tail[4];} world,expected;
#define RR_GPU_SCRATCH (&world.scratch)
static unsigned root_id,storage_id,packet_id;
static u32 seed=0x1b400;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static u8 *cursor(unsigned step){return (u8*)&world.storage[storage_id].packets[packet_id+step];}
static u32 *head(unsigned step){return &world.orders[root_id][step==0?146:step==1?133:147];}
static void record(struct World *v,void *packet,u32 a,u32 b,u32 c,u32 d,u32 e){
 Event *event;assert(v->calls<3);event=&v->log[v->calls++];
 event->cursor=v->scratch.cursor;event->order=v->scratch.order;event->packet=packet;
 event->args[0]=a;event->args[1]=b;event->args[2]=c;event->args[3]=d;event->args[4]=e;
}
static void change_roots(struct World *v,unsigned step){
 v->scratch.cursor=(u8*)&world.storage[(storage_id+step+1)%4].packets[10];
 v->scratch.order=world.orders[(root_id+step+1)%3];
}
''' + page + window + builder + r'''
void SetDrawMode(void *output,s32 display,s32 dither,s32 page,RrGpuRectangleFields *window){
 unsigned step=world.calls;
 assert(step==1 || step==2);assert(output==cursor(step));
 assert(display==0 && dither==1 && page==11 && window==0);
 assert(world.scratch.cursor==cursor(step));
 record(&world,output,display,dither,page,0,0);
 actual_builder(output,display,dither,page,window);
 /* Mutate the ordering head before the outer helper reads it. */
 *head(step)=0xAB345600U+step;
 change_roots(&world,step);
}
u8 *garage_add_draw_area(u32 *ordering,u8 *packet,u32 x,u32 y,u32 width,u32 height){
 assert(world.calls==0 && ordering==head(0) && packet==cursor(0));
 assert(x==12 && y==61 && width==296 && height==352);
 record(&world,packet,x,y,width,height,0);
 /* Unrecovered clip-builder boundary: preserve its verified three-word stride
  * while making writes and pointer changes observable to the caller. */
 ((RrGpuDrawModePacket *)packet)->tag.word=0x02123456;
 ((RrGpuDrawModePacket *)packet)->command[0]=0xE3001234;
 ((RrGpuDrawModePacket *)packet)->command[1]=0xE4005678;
 change_roots(&world,0);
 return packet+12;
}
''' + mode + setup + r'''
static void reference(void){
 unsigned step;RrGpuDrawModePacket *p=&expected.storage[storage_id].packets[packet_id];
 record(&expected,cursor(0),12,61,296,352,0);
 p[0].tag.word=0x02123456;p[0].command[0]=0xE3001234;p[0].command[1]=0xE4005678;
 change_roots(&expected,0);
 for(step=1;step<=2;step++){
  u32 *ordering=&expected.orders[root_id][step==1?133:147];
  expected.scratch.cursor=cursor(step);
  record(&expected,cursor(step),0,1,11,0,0);
  p[step].tag.word=(p[step].tag.word&0xffffffU)|0x02000000U;
  p[step].command[0]=0xE100020B;p[step].command[1]=0;
  *ordering=0xAB345600U+step;change_roots(&expected,step);
  p[step].tag.word=(p[step].tag.word&0xff000000U)|(*ordering&0xffffffU);
  *ordering=(*ordering&0xff000000U)|((uintptr_t)cursor(step)&0xffffffU);
 }
 expected.scratch.cursor=cursor(3);
}
int main(void){unsigned k,i;
 for(k=0;k<65536;k++){
  for(i=0;i<sizeof(world);i++)((u8*)&world)[i]=(u8)next();
  root_id=k%3;storage_id=(k>>2)%4;packet_id=(k>>4)%8;
  world.scratch.cursor=cursor(0);world.scratch.order=world.orders[root_id];world.calls=0;
  memcpy(&expected,&world,sizeof(world));reference();
  assert((uintptr_t)world.scratch.cursor>UINT32_MAX && (uintptr_t)world.scratch.order>UINT32_MAX);
  garage_add_editor_draw_env();
  assert(memcmp(&world,&expected,sizeof(world))==0);
 }
 return 0;
}
'''
        with TemporaryDirectory() as temp:
            p=Path(temp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
