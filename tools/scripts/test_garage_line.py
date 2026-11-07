"""Vertical LINE_F2 packet and serialized DMA links, with real owner aliasing."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

class Overlay662LineTest(unittest.TestCase):
    def test_packet_bits_links_aliases_and_native_cursor(self):
        root=Path(__file__).resolve().parents[2]
        # Remove only target register-placement metadata for the native compiler.
        body=re.sub(r' asm\("[^"]*"\)','',(root/'src/overlays/PAL/garage/func_8011AE2C.c').read_text())
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
#include "rr/overlay662_line.h"
static struct World {u32 head[4];RrGarageSelectionPrefix garage;RrGpuLineF2Packet packets[5];u32 order[4];u32 tail[4];} world,expected;
#define garage_logo_state world.garage
''' + body + r'''
static u32 seed=0x1AE2C;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static s16 half(u16 bits){s16 v;memcpy(&v,&bits,2);return v;}
static void coordinate(s16 *out,u32 value){u16 bits=(u16)value;memcpy(out,&bits,2);}
static u32 *ordering(struct World *v,unsigned mode,unsigned index){
 switch(mode){case 0:return &v->order[index%4];case 1:return &v->packets[index].tag.word;case 2:return &v->packets[index+1].tag.word;case 3:return &v->packets[index-1].tag.word;default:return &v->garage.phaseCC;}
}
static void reference(unsigned mode,unsigned index,u32 x,u32 y0,u32 y1,u32 r,u32 g,u32 b){
 RrGpuLineF2Packet *p=&expected.packets[index];u32 *ot=ordering(&expected,mode,index);
 u32 bx=(u16)expected.garage.halfC8,by=(u16)expected.garage.halfCA;
 p->tag.bytes.length=3;p->code=0x40;
 coordinate(&p->x0,bx+x);coordinate(&p->x1,bx+x);
 coordinate(&p->y0,by+y0);coordinate(&p->y1,by+y1);
 p->r=(u8)r;p->g=(u8)g;p->b=(u8)b;
 p->tag.word=(p->tag.word&0xFF000000U)|(*ot&0xFFFFFFU);
 *ot=(*ot&0xFF000000U)|((u32)(uintptr_t)&world.packets[index]&0xFFFFFFU);
}
int main(void){unsigned k,m,i;
 for(k=0;k<65536;k++)for(m=0;m<5;m++){
  u32 x=next(),y0=next(),y1=next(),r=next(),g=next(),b=next();unsigned index=1+(k&1);RrGpuLineF2Packet *result;
  for(i=0;i<sizeof(world);i++)((u8*)&world)[i]=(u8)next();
  world.garage.halfC8=half((u16)k);world.garage.halfCA=half((u16)(k*40503U));
  for(i=0;i<2;i++){world.garage.image38[i]=world.order;world.garage.image40[i]=world.order;world.garage.palette48[i]=world.order;}
  for(i=0;i<8;i++)world.garage.options78.flat[i]=0;
  if(k<8){static const u32 edges[]={0,1,0x7FFF,0x8000,0xFFFF,0x10000,0x7FFFFFFF,0xFFFFFFFF};x=edges[k];y0=edges[7-k];y1=edges[(k+3)%8];r=edges[k];g=edges[7-k];b=edges[(k+5)%8];}
  memcpy(&expected,&world,sizeof(world));reference(m,index,x,y0,y1,r,g,b);
  result=garage_add_vline(ordering(&world,m,index),&world.packets[index],x,y0,y1,r,g,b);
  assert(result==&world.packets[index+1]);assert((uintptr_t)result>UINT32_MAX);assert(memcmp(&world,&expected,sizeof(world))==0);
 }
 return 0;
}
'''
        with TemporaryDirectory() as temp:
            p=Path(temp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
