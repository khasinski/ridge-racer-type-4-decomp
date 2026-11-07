"""Serialized packet references, all legal ordering aliases and overlapping origins."""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest


class GaragePacketBuildersTest(unittest.TestCase):
    def test_native_packets_and_aliases(self):
        root = Path(__file__).resolve().parents[2]
        bodies = '\n'.join((root / f'src/overlays/PAL/garage/func_{a}.c').read_text()
                           for a in ('8011B1CC', '8011AD3C', '8011AEC4'))
        bodies = bodies.replace(' __asm__("$3")', '').replace(' __asm__("$10")', '')
        source = r'''
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4 || sizeof(t)==n,"PAL size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4 || offsetof(t,f)==n,"PAL offset")
#include "rr/overlay662_primitives.h"
typedef union {
 RrGpuPolyFT4Packet quads[3];RrGpuLineF4Packet frames[3];
 RrGpuTilePacket lines[3];
} PacketOwner;
typedef union {
 RrGarageSelectionPrefix state;
 struct {u8 pad[offsetof(RrGarageSelectionPrefix,halfC8)-8];PacketOwner storage;} view;
} GarageOwner;
static struct World {u32 guard[4];GarageOwner garage;PacketOwner storage;u32 ordering;u32 tail[4];} world,expected;
#define garage_logo_state world.garage.state
''' + bodies + r'''
static u32 seed=0x1b1cc;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static void put16(void *base,unsigned offset,u32 value){u8 *p=base;p[offset]=(u8)value;p[offset+1]=(u8)(value>>8);}
static void put32(void *base,unsigned offset,u32 value){u8 *p=base;unsigned i;for(i=0;i<4;i++)p[offset+i]=(u8)(value>>(8*i));}
static u32 get32(const void *base,unsigned offset){const u8 *p=base;return (u32)p[offset]|((u32)p[offset+1]<<8)|((u32)p[offset+2]<<16)|((u32)p[offset+3]<<24);}
static PacketOwner *storage(struct World *w,unsigned mode){return mode?&w->garage.view.storage:&w->storage;}
static u32 *ordering(struct World *w,unsigned kind,unsigned mode,unsigned which){
 PacketOwner *p=storage(w,mode);
 if(which>=3 && (kind!=2 || which>=6))return &w->ordering;
 if(kind==0)return &p->quads[which].tag.word;
 if(kind==1)return &p->lines[which].tag.word;
 return which<3?&p->frames[which].tag.word:&p->frames[which-3].pad;
}
static void link_packet(u8 *p,u8 *o,const void *actual_packet){
 put32(p,0,(get32(p,0)&0xff000000U)|(get32(o,0)&0xffffffU));
 put32(o,0,(get32(o,0)&0xff000000U)|((uintptr_t)actual_packet&0xffffffU));
}
static void reference(unsigned kind,unsigned mode,unsigned which,const u32 a[10]){
 u32 bx=(u16)expected.garage.state.halfC8,by=(u16)expected.garage.state.halfCA;
 PacketOwner *owner=storage(&expected,mode),*actual=storage(&world,mode);
 u8 *p=(u8*)owner,*o=(u8*)ordering(&expected,kind,mode,which);unsigned i;
 if(kind==0){
  p[3]=9;p[7]=0x2d;
  put16(p,8,bx+a[0]);put16(p,10,by+a[1]);
  put16(p,16,bx+a[0]+a[2]-1);put16(p,18,by+a[1]);
  put16(p,24,bx+a[0]);put16(p,26,by+a[1]+a[3]-1);
  put16(p,32,bx+a[0]+a[2]-1);put16(p,34,by+a[1]+a[3]-1);
  p[12]=(u8)a[4];p[13]=(u8)a[5];p[20]=(u8)(a[4]+a[6]-1);p[21]=(u8)a[5];
  p[28]=(u8)a[4];p[29]=(u8)(a[5]+a[7]-1);
  p[36]=(u8)(a[4]+a[6]-1);p[37]=(u8)(a[5]+a[7]-1);
  put16(p,14,a[8]);put16(p,22,a[9]);link_packet(p,o,&actual->quads[0]);
 }else if(kind==1){
  for(i=0;i<2;i++,p+=16){
   p[3]=3;p[7]=0x40;p[4]=(u8)a[3];p[5]=(u8)a[4];p[6]=(u8)a[5];
   put16(p,8,bx+a[0]);put16(p,10,by+a[1]+i);
   put16(p,12,bx+a[2]);put16(p,14,by+a[1]+i);
   link_packet(p,o,&actual->lines[i]);
  }
 }else{
  u32 x=bx+a[0],y=by+a[1],right=x+a[2],bottom=y+a[3];
  for(i=0;i<2;i++,p+=28){
   p[3]=6;p[7]=0x4c;put32(p,24,0x55555555U);
   p[4]=(u8)a[4];p[5]=(u8)a[5];p[6]=(u8)a[6];
   if(i==0){
    put16(p,8,x);put16(p,10,y);put16(p,12,right-1);put16(p,14,y);
    put16(p,16,right-1);put16(p,18,bottom-1);put16(p,20,x);put16(p,22,bottom-1);
   }else{
    put16(p,8,right-2);put16(p,10,y+1);put16(p,12,x);put16(p,14,y+1);
    put16(p,16,x);put16(p,18,bottom-2);put16(p,20,right-2);put16(p,22,bottom-2);
   }
   link_packet(p,o,&actual->frames[i]);
  }
 }
}
int main(void){
 unsigned k,i,kind,mode,which;u32 a[10];
 const u32 edge[]={0,1,0xffff,0x10000,0x7fffffff,0x80000000,0xffffffff,0xff,0x100,0x12345678,0x7fff,0x8000,0xffffff};
 u32 endian=1;assert(*(u8*)&endian==1);
 for(kind=0;kind<3;kind++)for(k=0;k<262144;k++){
  PacketOwner *p;u32 *o;
  for(i=0;i<sizeof(world);i++)((u8*)&world)[i]=(u8)next();
  mode=k&1;which=(k/2)%7;
  put16(&world.garage.state,offsetof(RrGarageSelectionPrefix,halfC8),k);
  put16(&world.garage.state,offsetof(RrGarageSelectionPrefix,halfCA),k*40503U+17U);
  for(i=0;i<10;i++)a[i]=(k&4)?next():edge[(k+i)%13];
  memcpy(&expected,&world,sizeof(world));reference(kind,mode,which,a);
  p=storage(&world,mode);o=ordering(&world,kind,mode,which);assert((uintptr_t)p>UINT32_MAX);
  if(kind==0)assert(garage_add_textured_rect(o,p->quads,a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7],a[8],a[9])==&p->quads[1]);
  else if(kind==1)assert(garage_add_hline_double(o,p->lines,a[0],a[1],a[2],a[3],a[4],a[5])==&p->lines[2]);
  else assert(garage_add_rect_outline(o,p->frames,a[0],a[1],a[2],a[3],a[4],a[5],a[6])==&p->frames[2]);
  assert(memcmp(&world,&expected,sizeof(world))==0);
 }
 return 0;
}
'''
        with TemporaryDirectory() as directory:
            p = Path(directory)
            (p / 'test.c').write_text(source)
            subprocess.run(['cc', '-std=gnu11', '-O2', '-fsanitize=undefined',
                            '-fno-sanitize-recover=all', '-I', str(root / 'include'),
                            str(p / 'test.c'), '-o', str(p / 'test')], check=True)
            subprocess.run([str(p / 'test')], check=True)


if __name__ == '__main__':
    unittest.main()
