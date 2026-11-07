"""Serialized DMA links, overlapping ordering words and captured garage origin."""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest

class GarageTileTest(unittest.TestCase):
    def test_native_packets_and_serialized_dma_links(self):
        root=Path(__file__).resolve().parents[2]
        body=(root/'src/overlays/PAL/garage/func_8011B034.c').read_text()
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
typedef union {RrGpuTilePacket packets[3];u32 words[12];} PacketOwner;
typedef union {RrGarageSelectionPrefix state;struct {u8 pad[offsetof(RrGarageSelectionPrefix,halfC8)-8];PacketOwner storage;} view;} GarageOwner;
static struct World {u32 guard[4];GarageOwner garage;PacketOwner storage;u32 ordering;u32 tail[4];} world,e;
#define garage_logo_state world.garage.state
''' + body + r'''
static u32 seed=0x1b034;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static void put16(void *base,unsigned offset,u32 value){u8 *p=base;p[offset]=(u8)value;p[offset+1]=(u8)(value>>8);}
static void put32(void *base,unsigned offset,u32 value){u8 *p=base;unsigned i;for(i=0;i<4;i++)p[offset+i]=(u8)(value>>(8*i));}
static u32 get32(const void *base,unsigned offset){const u8 *p=base;return (u32)p[offset]|((u32)p[offset+1]<<8)|((u32)p[offset+2]<<16)|((u32)p[offset+3]<<24);}
static PacketOwner *storage(struct World *v,unsigned mode){return mode?&v->garage.view.storage:&v->storage;}
static u32 *order(struct World *v,unsigned mode,unsigned which){if(which==12)return &v->ordering;return &storage(v,mode)->words[which];}
static void reference(unsigned mode,unsigned which,const u32 a[7]){
 u16 bx=(u16)e.garage.state.halfC8,by=(u16)e.garage.state.halfCA;
 u8 *p=(u8*)&storage(&e,mode)->packets[0];
 u8 *o=(u8*)order(&e,mode,which);
 p[3]=3;p[7]=0x60;
 put16(p,8,(u32)bx+a[0]);put16(p,10,(u32)by+a[1]);put16(p,12,a[2]);put16(p,14,a[3]);
 p[4]=(u8)a[4];p[5]=(u8)a[5];p[6]=(u8)a[6];
 put32(p,0,(get32(p,0)&0xff000000U)|(get32(o,0)&0xffffffU));
 /* Use the actual packet address, preserving all pointer bits until encoding
  * the low24 hardware link. No serialized word is dereferenced as a pointer. */
 put32(o,0,(get32(o,0)&0xff000000U)|((uintptr_t)&storage(&world,mode)->packets[0]&0xffffffU));
}
int main(void){unsigned k,i,mode,which;const u32 edge[]={0,1,0xffff,0x10000,0x7fffffff,0x80000000,0xffffffff,0xff,0x100,0x12345678,0x7fff,0x8000,0xffffff};u32 args[7];
 u32 endian=1;assert(*(u8*)&endian==1);
 for(k=0;k<262144;k++){
  RrGpuTilePacket *returned,*packet;
  for(i=0;i<sizeof(world);i++)((u8*)&world)[i]=(u8)next();
  mode=k&1;which=(k/2)%13;
  put16(&world.garage.state,offsetof(RrGarageSelectionPrefix,halfC8),k);
  put16(&world.garage.state,offsetof(RrGarageSelectionPrefix,halfCA),k*40503U+17U);
  for(i=0;i<7;i++)args[i]=(k&4)?next():edge[(k+i)%13];
  memcpy(&e,&world,sizeof(world));reference(mode,which,args);
  packet=&storage(&world,mode)->packets[0];assert((uintptr_t)packet>UINT32_MAX);
  returned=garage_add_tile(order(&world,mode,which),packet,args[0],args[1],args[2],args[3],args[4],args[5],args[6]);
  assert(returned==&storage(&world,mode)->packets[1] && (uintptr_t)returned>UINT32_MAX);
  assert(memcmp(&world,&e,sizeof(world))==0);
 }
 return 0;
}
'''
        with TemporaryDirectory() as temp:
            p=Path(temp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
