"""POLY_G4 semantic/DMA views, wrapped geometry and actual word aliases."""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest

class GarageGradientQuadTest(unittest.TestCase):
    def test_geometry_colors_links_and_complete_packet_owners(self):
        root=Path(__file__).resolve().parents[2]
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
#include "rr/garage_gradient.h"
static struct World {u32 head[4];RrGarageSelectionPrefix state;RrPolyG4Storage packets[5];u32 ordering[4];u32 tail[4];} world,expected;
#define garage_logo_state world.state
''' + (root/'src/overlays/PAL/garage/func_8011B0CC.c').read_text() + r'''
static u32 seed=0x1B0CC;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static s16 half(u32 raw){u16 bits=(u16)raw;s16 v;memcpy(&v,&bits,2);return v;}
static void put16(void *base,unsigned offset,u16 bits){u8 *p=base;p[offset]=(u8)bits;p[offset+1]=(u8)(bits>>8);}
static u32 get32(const void *base){const u8 *p=base;return (u32)p[0]|((u32)p[1]<<8)|((u32)p[2]<<16)|((u32)p[3]<<24);}
static void put32(void *base,u32 bits){u8 *p=base;unsigned i;for(i=0;i<4;i++)p[i]=(u8)(bits>>(8*i));}
static u32 *ordering(struct World *v,unsigned mode,unsigned index){
 if(mode==0)return &v->ordering[index%4];
 if(mode<=9)return &v->packets[index].words[mode-1];
 if(mode==10)return &v->packets[index-1].words[0];
 if(mode==11)return &v->packets[index+1].words[0];
 return &v->state.phaseCC;
}
static void reference(unsigned mode,unsigned index,const u32 a[10]){
 unsigned i;u8 *p=(u8*)&expected.packets[index];u32 *ot=ordering(&expected,mode,index);
 uint64_t x=(u16)expected.state.halfC8,y=(u16)expected.state.halfCA;
 u16 left=(u16)(x+a[0]),top=(u16)(y+a[1]),right=(u16)(x+a[0]+a[2]-1ULL),bottom=(u16)(y+a[1]+a[3]-1ULL);
 p[3]=8;p[7]=0x38;
 put16(p,8,left);put16(p,10,top);put16(p,16,right);put16(p,18,top);
 put16(p,24,left);put16(p,26,bottom);put16(p,32,right);put16(p,34,bottom);
 for(i=0;i<3;i++){p[4+i]=(u8)a[4+i];p[20+i]=(u8)a[4+i];p[12+i]=(u8)a[7+i];p[28+i]=(u8)a[7+i];}
 put32(p,(get32(p)&0xFF000000U)|(*ot&0xFFFFFFU));
 *ot=(*ot&0xFF000000U)|((uintptr_t)&world.packets[index]&0xFFFFFFU);
}
int main(void){unsigned k,mode,i,index;u32 a[10];
 _Static_assert(sizeof(RrPolyG4Storage)==36,"one complete POLY_G4 owner");
 for(k=0;k<65536;k++)for(mode=0;mode<13;mode++){
  RrPolyG4Storage *returned;index=1+(k&1);
  for(i=0;i<sizeof(world);i++)((u8*)&world)[i]=(u8)next();
  world.state.halfC8=half(k);world.state.halfCA=half(k*40503U);
  for(i=0;i<2;i++){world.state.image38[i]=world.ordering;world.state.image40[i]=world.ordering;world.state.palette48[i]=world.ordering;}
  for(i=0;i<8;i++)world.state.options78.flat[i]=0;
  for(i=0;i<10;i++)a[i]=next();
  if(mode==0){a[0]&=0xFFFF0000U;a[1]&=0xFFFF0000U;}
  a[2]=(a[2]&0xFFFF0000U)|k;a[3]=(a[3]&0xFFFF0000U)|((k*40503U)&0xFFFFU);
  if(k<8){static const u32 edge[8]={0,1,0x7FFF,0x8000,0xFFFF,0x10000,0x80000000U,0xFFFFFFFFU};for(i=0;i<10;i++)a[i]=edge[(k+i)%8];}
  memcpy(&expected,&world,sizeof(world));reference(mode,index,a);
  returned=garage_add_gradient_rect(ordering(&world,mode,index),&world.packets[index],a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7],a[8],a[9]);
  assert(returned==&world.packets[index+1] && (uintptr_t)returned>UINT32_MAX);assert(memcmp(&world,&expected,sizeof(world))==0);
 }
 return 0;
}
'''
        with TemporaryDirectory() as temp:
            p=Path(temp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
