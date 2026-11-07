"""Draw-mode list insertion and the real resident helper closure, with full pointers."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

class GarageDrawModeTest(unittest.TestCase):
    def test_actual_helpers_and_live_packet_words(self):
        root=Path(__file__).resolve().parents[2]
        mode=(root/'src/overlays/PAL/garage/func_8011AC94.c').read_text()
        page=(root/'tools/scripts/fixtures/views/func_800940E0.c').read_text()
        # Register allocation metadata has no native architectural equivalent.
        page=re.sub(r' asm\("\$\d+"\)','',page)
        window=(root/'tools/scripts/fixtures/views/func_8009424C.c').read_text()
        builder=(root/'tools/scripts/fixtures/views/func_80093C24.c').read_text()
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
typedef union {RrGpuDrawModePacket packets[4];u32 words[12];RrGpuRectangleFields windows[6];} Owner;
static struct World {u32 guard[4];Owner storage;RrGpuRectangleFields window;u32 order;u32 tail[4];} world,expected;
''' + page + window + builder + mode + r'''
static u32 seed=0x1ac94;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static void put32(void *p,unsigned offset,u32 value){u8 *b=p;unsigned i;for(i=0;i<4;i++)b[offset+i]=(u8)(value>>(8*i));}
static u32 get32(const void *p,unsigned offset){const u8 *b=p;return (u32)b[offset]|((u32)b[offset+1]<<8)|((u32)b[offset+2]<<16)|((u32)b[offset+3]<<24);}
static u32 *ordering(struct World *v,unsigned index){return index==12?&v->order:&v->storage.words[index];}
static RrGpuRectangleFields *texture_window(struct World *v,unsigned index){
 if(index==0)return 0;if(index==1)return &v->window;return &v->storage.windows[index-2];
}
static s32 signed_word(u32 raw){s32 value;memcpy(&value,&raw,4);return value;}
static void ref_builder(s32 display,s32 dither,u32 page,unsigned win){
 u8 *p=(u8*)&expected.storage;u32 command=0xe1000000U | (page&0x9ffU);RrGpuRectangleFields *window;
 if(display)command|=0x400;if(dither)command|=0x200;
 p[3]=2;put32(p,4,command);
 window=texture_window(&expected,win);command=0;
 if(window){
  command=0xe2000000U | (((u32)(u8)window->y>>3)<<15) | (((u32)(u8)window->x>>3)<<10)
   | (((u32)(-window->h)&255U)>>3)*32U | (((u32)(-window->w)&255U)>>3);
 }
 put32(p,8,command);
}
int main(void){unsigned k,i,where,win;u32 x,y;
 for(k=0;k<2097152;k++){
  RrGpuDrawModePacket *returned;u8 *p,*o;u32 page;
  for(i=0;i<sizeof(world);i++)((u8*)&world)[i]=(u8)next();
  x=(k&1023U);y=(k>>10)&1023U;
  if(k>=1048576){x|=next()&0xfffffc00U;y|=next()&0xfffffc00U;}
  where=k%13;memcpy(&expected,&world,sizeof(world));
  page=((y&0x100U)/16U)|((x&0x3ffU)/64U)|((y&0x200U)*4U);
  ref_builder(0,1,page,0);p=(u8*)&expected.storage;o=(u8*)ordering(&expected,where);
  put32(p,0,(get32(p,0)&0xff000000U)|(get32(o,0)&0xffffffU));
  put32(o,0,(get32(o,0)&0xff000000U)|((uintptr_t)&world.storage.packets[0]&0xffffffU));
  returned=garage_add_draw_mode(ordering(&world,where),&world.storage.packets[0],x,y);
  assert(returned==&world.storage.packets[1] && (uintptr_t)returned>UINT32_MAX);
  assert(memcmp(&world,&expected,sizeof(world))==0);
 }
 /* Canonical pointer ABI: real texture-window helper with external and live
  * overlapping windows, so a truncated or fake integer argument cannot pass. */
 for(k=0;k<131072;k++){
  s32 display,dither,page;
  for(i=0;i<sizeof(world);i++)((u8*)&world)[i]=(u8)next();
  display=(k&1)?signed_word(next()):0;dither=(k&2)?signed_word(next()):0;page=signed_word(next());win=k%8;
  memcpy(&expected,&world,sizeof(world));ref_builder(display,dither,(u32)page&0xffffU,win);
  if(texture_window(&world,win))assert((uintptr_t)texture_window(&world,win)>UINT32_MAX);
  SetDrawMode(&world.storage.packets[0],display,dither,page,texture_window(&world,win));
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
