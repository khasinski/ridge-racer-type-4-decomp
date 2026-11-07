"""Actual GPU fill builder with both packet branches and all contained rectangle aliases."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

ROOT=Path(__file__).resolve().parents[2]


class GpuFillPacketsTest(unittest.TestCase):
    def test_actual_body_and_packet_mutations(self):
        source=(ROOT/'tools/scripts/fixtures/views/func_800943C4.c').read_text()
        source=re.sub(r'^#include.*\n','',source,flags=re.M)
        source=re.sub(r'^extern u32 D_80112788.*\n','',source,flags=re.M)
        source=re.sub(r' asm\("\$\d+"\)','',source)
        source=re.sub(r'__asm__\(""[^;]*;',';',source)
        prefix=r'''

#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,s)
#define RR_ASSERT_OFFSET(t,f,o)
#include "rr/gpu.h"
_Static_assert(sizeof(RrGpuRectangle)==8,"rectangle");
_Static_assert(offsetof(RrGpuRectangleFields,w)==4,"width");
_Static_assert(offsetof(RrGpuRectangleFields,h)==6,"height");
_Static_assert(sizeof(unsigned long)>=sizeof(void*),"native pointers");
typedef union {u32 words[10];RrGpuRectangle rectangles[5];struct {u32 lead;RrGpuRectangle rectangles[4];u32 tail;} odd;} Primary;
typedef union {u32 words[4];RrGpuRectangle rectangles[2];struct {u32 lead;RrGpuRectangle rectangle;u32 tail;} odd;} Secondary;
typedef struct {u32 before[4]; RrGpuRectangle rectangle;Primary primary;Secondary secondary;u32 status;s16 width,height;u32 after[4];} World;
typedef struct {int kind;unsigned long arg;World state;} Entry;
static World initial,world,actual;static Entry entries[4],saved[4];
static unsigned count,mutate,selection,seed=0xC35A4129;
#define D_80112788 (world.primary.words)
#define D_801127B0 (world.secondary.words)
#define D_800ABAC4 (world.width)
#define GpuVramHeight (world.height)
#define GPU_STATUS (&world.status)
_Static_assert(sizeof(Primary)==40,"primary owner");
_Static_assert(sizeof(Secondary)==16,"secondary owner");
_Static_assert(offsetof(Primary,odd.rectangles)==4,"odd primary view");
_Static_assert(offsetof(Secondary,odd.rectangle)==4,"odd secondary view");
static RrGpuRectangle *rectangle(void){
 if(selection==0)return &world.rectangle;
 if(selection<=9){unsigned n=selection-1;return (n&1)?&world.primary.odd.rectangles[n/2]:&world.primary.rectangles[n/2];}
 unsigned n=selection-10;
 return n==1?&world.secondary.odd.rectangle:&world.secondary.rectangles[n/2];
}
static u32 random32(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static void event(int kind,unsigned long arg){
 assert(count<4);Entry *e=&entries[count++];memset(e,0,sizeof(*e));e->kind=kind;e->arg=arg;e->state=world;
 if(mutate){
  for(unsigned i=0;i<10;i++)world.primary.words[i]^=0x28193273U*(count+i);
  for(unsigned i=0;i<4;i++)world.secondary.words[i]+=0xF1938351U*(count+i);
  world.rectangle.words[0]^=0x7123ABCDU;world.rectangle.words[1]+=0x90127312U;
  world.width=(s16)(u16)((u16)world.width+0x8231U);world.height=(s16)(u16)((u16)world.height+0x73U);
  world.status^=0x31578321U;
 }
}
s32 _param(s32 command){
 assert(command==3+(int)count);event(0,(unsigned)command);
 return (s32)((0x782301U*(unsigned)command ^ world.status)&0xFFFFFFU);
}
void _cwc(unsigned long address){assert(address==(unsigned long)world.primary.words);event(1,address);}
static u16 get16(RrGpuRectangle *r,unsigned off){u16 value;memcpy(&value,(u8*)r+off,2);return value;}
static int signed16(u16 v){return v<32768?(int)v:(int)v-65536;}
static u32 get32(RrGpuRectangle *r,unsigned off){u32 value;memcpy(&value,(u8*)r+off,4);return value;}
static void put16(RrGpuRectangle *r,unsigned off,u16 value){memcpy((u8*)r+off,&value,2);}
static int reference(RrGpuRectangle *r,u32 color){
 for(unsigned off=4;off<=6;off+=2){
  int side=signed16(get16(r,off));int cap=off==4?world.width:world.height;
  u16 clamped=side<0?0:side>cap-1?(u16)((u16)cap-1U):(u16)side;
  put16(r,off,clamped);
 }
 u32 *p=world.primary.words,*q=world.secondary.words;
 if((get16(r,0)%64)!=0 || (get16(r,4)%64)!=0){
  p[0]=0x08000000U|((uintptr_t)q&0xFFFFFFU);
  p[4]=0xE6000000U;p[1]=0xE3000000U;p[2]=0xE4FFFFFFU;p[3]=0xE5000000U;
  u32 status=world.status;
  p[6]=0x60000000U|(color&0xFFFFFFU);
  p[5]=0xE1000000U|(status&0x7FFU)|((color>>31)*1024U);
  p[7]=get32(r,0);u32 size=get32(r,4);q[0]=0x03FFFFFFU;p[8]=size;
  q[1]=(u32)_param(3)|0xE3000000U;
  q[2]=(u32)_param(4)|0xE4000000U;
  q[3]=(u32)_param(5)|0xE5000000U;
 }else{
  p[0]=0x05FFFFFFU;p[1]=0xE6000000U;u32 status=world.status;
  p[3]=0x02000000U|(color&0xFFFFFFU);
  p[2]=0xE1000000U|(status&0x7FFU)|((color>>31)*1024U);
  p[4]=get32(r,0);p[5]=get32(r,4);
 }
 _cwc((unsigned long)p);return 0;
}
s32 _clr(RrGpuRectangle *,u32);
int main(void){
 unsigned fast=0,extended=0;
 assert((uintptr_t)world.primary.words>UINT32_MAX);
 for(unsigned k=0;k<1703936;k++){
  u8 *bytes=(u8*)&initial;for(size_t i=0;i<sizeof(initial);i++)bytes[i]=(u8)random32();
  selection=k/131072U;mutate=(k>>16)&1;u32 color=random32();
  world=initial;RrGpuRectangle *r=rectangle();
  r->rect.w=(s16)(u16)k;r->rect.h=(s16)(u16)(k*31337U);
  world.width=(s16)(u16)(k*139U);world.height=(s16)(u16)(k*17U);
  if((k&7)==0){r->rect.x=(s16)(u16)(k*64U);world.width=32767;}
  initial=world;count=0;memset(entries,0,sizeof(entries));
  assert(_clr(rectangle(),color)==0);actual=world;unsigned calls=count;
  memcpy(saved,entries,sizeof(saved));if(calls==1)fast++;else {assert(calls==4);extended++;}
  world=initial;count=0;memset(entries,0,sizeof(entries));
  assert(reference(rectangle(),color)==0);assert(count==calls);
  assert(!memcmp(saved,entries,sizeof(saved)));assert(!memcmp(&actual,&world,sizeof(world)));
 }
 assert(fast && extended);puts("1703936 native fill-packet cases passed");
}
'''
        with TemporaryDirectory() as directory:
            c=Path(directory)/'fill.c'
            exe=Path(directory)/'fill'
            c.write_text(prefix+'\n'+source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined',
                            '-fno-sanitize-recover=all','-I',str(ROOT/'include'),
                            str(c),'-o',str(exe)],check=True,capture_output=True,text=True)
            result=subprocess.run([str(exe)],check=True,capture_output=True,text=True)
            self.assertIn('1703936 native fill-packet cases passed',result.stdout)
