"""Sprite/clip orchestration with genuine scratchpad pointers and ABI boundaries."""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest

class GarageSpriteClipTest(unittest.TestCase):
    def test_gate_captured_order_and_cursor_returns(self):
        root=Path(__file__).resolve().parents[2]
        body=(root/'src/overlays/PAL/garage/func_8011B8B4.c').read_text()
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
#include "rr/overlay662_primitives.h"
#include "rr/animated_sprite_grid.h"
typedef union {u8 bytes[128];u32 words[32];RrSpritePacket packets[6];} Buffer;
typedef struct {u8 *cursor,*published;u32 *ordering,*root;u32 args[6];} Event;
static struct World {u32 guard[4];RrGpuScratchPointers scratch;RrGarageWorkingStorage work;u32 orders[3][704];Buffer buffers[3];u32 calls;Event events[2];u32 tail[4];} world,expected;
#define RR_GPU_SCRATCH (&world.scratch)
#define garage_D_80127668 world.work
static unsigned root_id,buffer_id,offset;
static u32 seed=0x1b8b4;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static s16 half(u32 raw){u16 bits=raw;s16 result;memcpy(&result,&bits,2);return result;}
static u8 *cursor(unsigned step){return world.buffers[buffer_id].bytes+offset+step;}
static void record(struct World *v,u8 *packet,u32 *ordering,u32 a,u32 b,u32 c,u32 d){
 Event *event;assert(v->calls<2);event=&v->events[v->calls++];
 event->cursor=packet;event->published=v->scratch.cursor;event->ordering=ordering;event->root=v->scratch.order;
 event->args[0]=a;event->args[1]=b;event->args[2]=c;event->args[3]=d;
}
static void changes(struct World *v,unsigned step){
 v->scratch.cursor=world.buffers[(buffer_id+step+1)%3].bytes+64;
 v->scratch.order=world.orders[(root_id+step+1)%3];
 v->work.words[0]^=0x13579BDFU;v->work.words[14]=0xffffffffU;v->work.halves[10]=-123;v->work.halves[11]=456;
}
/* ABI-confirmed unrecovered helper boundaries. Their packet strides are20/12;
 * adversarial mutations make any unintended caller reload observable. */
u8 *garage_func_8011B2D4(s32 index,u8 *packet,s32 x,s32 y){
 unsigned i;assert(world.calls==0 && index>=0 && index<72 && packet==cursor(0));
 record(&world,packet,0,(u32)index,(u32)x,(u32)y,0);
 for(i=0;i<20;i++)packet[i]=(u8)(i*19U+(u32)index+(u32)x+(u32)y);
 changes(&world,0);return packet+20;
}
u8 *garage_func_8011ABB4(u32 *ordering,u8 *packet,u32 x,u32 y,u32 width,u32 height){
 unsigned i;assert(world.calls==1 && ordering==world.orders[root_id]+132 && packet==cursor(20));
 assert(world.scratch.cursor==packet && x==0 && y==0 && width==320 && height==480);
 record(&world,packet,ordering,x,y,width,height);
 for(i=0;i<12;i++)packet[i]=(u8)(i*23+7);
 *ordering=0xABCDEF01U;changes(&world,1);return packet+12;
}
''' + body + r'''
static void reference(void){
 unsigned i;u32 index=expected.work.words[14];s32 x=expected.work.halves[10],y=expected.work.halves[11];
 u8 *p=expected.buffers[buffer_id].bytes+offset;
 if(expected.work.words[0]&1)return;
 record(&expected,cursor(0),0,index,(u32)x,(u32)y,0);
 for(i=0;i<20;i++)p[i]=(u8)(i*19U+index+(u32)x+(u32)y);
 changes(&expected,0);expected.scratch.cursor=cursor(20);
 record(&expected,cursor(20),world.orders[root_id]+132,0,0,320,480);
 for(i=0;i<12;i++)p[20+i]=(u8)(i*23+7);
 expected.orders[root_id][132]=0xABCDEF01U;changes(&expected,1);expected.scratch.cursor=cursor(32);
}
int main(void){unsigned k,i;
 for(k=0;k<262144;k++){
  for(i=0;i<sizeof(world);i++)((u8*)&world)[i]=(u8)next();
  root_id=k%3;buffer_id=(k>>2)%3;offset=((k>>4)&7)*4;
  world.scratch.cursor=cursor(0);world.scratch.order=world.orders[root_id];world.calls=0;
  world.work.words[0]=(next()&~1U)|(k&1U);
  world.work.words[14]=(k&1U)?next():((k>>1)%72);
  world.work.halves[10]=half(k>>1);world.work.halves[11]=half((k>>1)*40503U+17U);
  memcpy(&expected,&world,sizeof(world));reference();
  assert((uintptr_t)world.scratch.cursor>UINT32_MAX && (uintptr_t)world.scratch.order>UINT32_MAX);
  garage_func_8011B8B4();assert(memcmp(&world,&expected,sizeof(world))==0);
 }
 return 0;
}
'''
        with TemporaryDirectory() as temp:
            p=Path(temp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
