"""GPU upload body: access-by-access MMIO trace, mutable ports and bounded DMA owners."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

ROOT=Path(__file__).resolve().parents[2]


class GpuUploadTest(unittest.TestCase):
    def test_actual_body_and_mmio_trace(self):
        source=(ROOT/'tools/scripts/fixtures/views/func_800945F4.c').read_text()
        source=re.sub(r'^#include.*\n','',source,flags=re.M)
        source=re.sub(r' asm\("\$\d+"\)','',source)
        source=re.sub(r'__asm__\s*(?:volatile\s*)?\(""[^;]*;',';',source)
        # MMIO pointer/pointee type substitution only. Keep every original
        # load/store, loop, pointer increment and32-bit DMA wire conversion.
        source=source.replace('volatile u32 *','volatile GpuWord *')
        source=source.replace('*(u32 *)GPU_DATA','*(GpuWord *)GPU_DATA')
        prefix=r'''

#include <cassert>
#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,s)
#define RR_ASSERT_OFFSET(t,f,o)
#include "rr/gpu.h"
static u32 read_register(unsigned,unsigned);
static void write_register(unsigned,unsigned,u32);
struct GpuWord {
 u32 value;unsigned bank,kind;
 operator u32() volatile {return read_register(bank,kind);}
 void operator=(u32 v) volatile {write_register(bank,kind,v);}
};
struct World {
 u32 before[4];RrGpuRectangle rect;s16 width,height;
 GpuWord banks[2][5];unsigned selected[5];u32 pixels[2048];
 unsigned ready_after,reads,timeout_after,calls,mutation; s32 failure;u32 after[4];
};
struct Entry {unsigned kind,bank,reg;u32 value;World snapshot;};
static World initial,world,actual;static Entry entries[64],saved[64];
static unsigned count,seed=0x2A4EF139;
static_assert(sizeof(RrGpuRectangle)==8,"rectangle");
static_assert(offsetof(RrGpuRectangleFields,w)==4,"width");
static_assert(offsetof(RrGpuRectangleFields,h)==6,"height");
static_assert(sizeof(unsigned long)>=sizeof(void*),"full native pointers");
#define D_800ABAC4 (world.width)
#define GpuVramHeight (world.height)
#define GPU_DATA (&world.banks[world.selected[0]][0])
#define GPU_STATUS (&world.banks[world.selected[1]][1])
#define DMA2_MADR (&world.banks[world.selected[2]][2])
#define DMA2_BCR (&world.banks[world.selected[3]][3])
#define DMA2_CHCR (&world.banks[world.selected[4]][4])
static void record(unsigned kind,unsigned bank=0,unsigned reg=0,u32 value=0){
 assert(count<64);Entry *e=&entries[count++];std::memset(e,0,sizeof(*e));
 e->kind=kind;e->bank=bank;e->reg=reg;e->value=value;std::memcpy(&e->snapshot,&world,sizeof(world));
}
static u32 read_register(unsigned bank,unsigned reg){
 assert(reg==1);
 u32 value=(world.banks[bank][reg].value&~0x04000000U)|(world.reads>=world.ready_after?0x04000000U:0);
 record(0,bank,reg,value);world.reads++;return value;
}
static void write_register(unsigned bank,unsigned reg,u32 value){
 record(1,bank,reg,value);world.banks[bank][reg].value=value;
 if(world.mutation){
  for(unsigned i=0;i<5;i++)world.selected[i]^=1;
  world.rect.words[0]^=0x13579321U;world.rect.words[1]^=0x31297513U;
  for(unsigned i=0;i<16;i++)world.pixels[i]+=(value^0x71839127U)*(i+1U);
 }
}
static void call_mutation(bool initialize){
 if(!world.mutation)return;
 for(unsigned i=0;i<5;i++)world.selected[i]^=1;
 world.rect.rect.x=(s16)(u16)((u16)world.rect.rect.x+0x7181U);
 world.rect.rect.y=(s16)(u16)((u16)world.rect.rect.y+0x93A7U);
 if(initialize){
  world.rect.rect.w=(s16)(u16)((u16)world.rect.rect.w+0x81U);
  world.rect.rect.h=(s16)(u16)((u16)world.rect.rect.h+0x1731U);
  world.width=world.width<0?-1:1024;world.height=world.height<0?1:4;
 }else{
  world.rect.words[1]^=0x81397129U;
  world.width=(s16)(u16)((u16)world.width+0x9173U);
  world.height=(s16)(u16)((u16)world.height+0x31U);
 }
 for(unsigned i=0;i<16;i++)world.pixels[i]^=0x98327141U*(i+1U);
}
void set_alarm(void){record(2);call_mutation(true);}
s32 get_alarm(void){record(3);world.calls++;call_mutation(false);return world.calls==world.timeout_after?world.failure:0;}
static int reference(void){
 set_alarm();
 int width=world.rect.rect.w,height=world.rect.rect.h;
 if(width<0)width=0;else if(width>world.width)width=world.width;
 world.rect.rect.w=(s16)width;
 if(height<0)height=0;else if(height>world.height)height=world.height;
 world.rect.rect.h=(s16)height;
 int words=(width*height+1)/2;
 if(words<=0)return -1;
 assert(words<=2048);
 unsigned blocks=(unsigned)words/16U,tail=(unsigned)words%16U;
 while(!(read_register(world.selected[1],1)&0x04000000U))if(get_alarm()!=0)return -1;
 write_register(world.selected[1],1,0x04000000U);
 write_register(world.selected[0],0,0x01000000U);
 write_register(world.selected[0],0,0xA0000000U);
 write_register(world.selected[0],0,world.rect.words[0]);
 write_register(world.selected[0],0,world.rect.words[1]);
 for(unsigned i=0;i<tail;i++)write_register(world.selected[0],0,world.pixels[i]);
 if(blocks){
  write_register(world.selected[1],1,0x04000002U);
  write_register(world.selected[2],2,(u32)(uintptr_t)&world.pixels[tail]);
  write_register(world.selected[3],3,(blocks<<16)|16U);
  write_register(world.selected[4],4,0x01000201U);
 }
 return 0;
}
s32 _dws(RrGpuRectangle *,const u32 *);
static u32 random32(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
int main(void){
 static const s16 widths[]={-32768,-1,0,1,2,31,32,33,1024,32767};
 static const s32 failures[]={-1,1,INT32_MIN,INT32_MAX};
 unsigned passed=0,failed=0,bulk=0,immediate=0;
 assert((uintptr_t)world.pixels>UINT32_MAX);
 for(unsigned k=0;k<132096;k++){
  u8 *raw=(u8*)&initial;for(unsigned j=0;j<sizeof(initial);j++)raw[j]=(u8)random32();
  for(unsigned b=0;b<2;b++)for(unsigned j=0;j<5;j++){initial.banks[b][j].bank=b;initial.banks[b][j].kind=j;}
  for(unsigned j=0;j<5;j++)initial.selected[j]&=1;
  initial.rect.rect.w=(s16)(u16)k;initial.rect.rect.h=(s16)(u16)(k*31337U+0x1234U);
  initial.width=widths[k%10];initial.height=(s16)(k%5);
  if(initial.width>1024)initial.height=0; /* All actual transfer buffers remain in-bounds. */
  if((k&31)==15){initial.height=(s16)(u16)(k*139U);initial.width=initial.height>1024?0:4;}
  if(k>=131072){unsigned n=k-131072;initial.rect.rect.w=1+n%32;initial.rect.rect.h=1+n/32;initial.width=1024;initial.height=32;}
  initial.reads=initial.calls=0;initial.ready_after=k%4;initial.timeout_after=(k&4)?1+k%3:100;
  initial.failure=failures[(k>>3)&3];initial.mutation=(k>>16)&1;
  std::memcpy(&world,&initial,sizeof(world));count=0;std::memset(entries,0,sizeof(entries));
  int result=_dws(&world.rect,world.pixels);std::memcpy(&actual,&world,sizeof(world));
  unsigned calls=count;std::memcpy(saved,entries,sizeof(saved));
  if(result==0){passed++;bool dma=false;for(unsigned i=0;i<calls;i++)if(entries[i].kind==1 && entries[i].reg==4)dma=true;if(dma)bulk++;else immediate++;}else failed++;
  std::memcpy(&world,&initial,sizeof(world));count=0;std::memset(entries,0,sizeof(entries));
  int expected=reference();
  assert(result==expected);assert(count==calls);assert(!std::memcmp(saved,entries,sizeof(saved)));assert(!std::memcmp(&actual,&world,sizeof(world)));
 }
 assert(passed>1000 && failed>1000 && bulk>100 && immediate>100);
 std::puts("132096 native GPU-upload cases passed");
}
'''
        with TemporaryDirectory() as directory:
            cpp=Path(directory)/'gpu.cpp'
            exe=Path(directory)/'gpu'
            cpp.write_text(prefix+'\n'+source)
            subprocess.run(['c++','-std=c++14','-O2','-Wno-deprecated-register',
                            '-fsanitize=undefined','-fno-sanitize-recover=all',
                            '-I',str(ROOT/'include'),str(cpp),'-o',str(exe)],
                           check=True,capture_output=True,text=True)
            result=subprocess.run([str(exe)],check=True,capture_output=True,text=True)
            self.assertIn('132096 native GPU-upload cases passed',result.stdout)
