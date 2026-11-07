"""Actual fill-selecting environment builder and real texture-window helper."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

ROOT=Path(__file__).resolve().parents[2]


class DrawFillEnvironmentTest(unittest.TestCase):
    def test_actual_body_and_window_helper(self):
        source=(ROOT/'tools/scripts/fixtures/views/func_80093E70.c').read_text()
        source=re.sub(r'^#include.*\n','',source,flags=re.M)
        source=re.sub(r' asm\("\$\d+"\)','',source)
        source=re.sub(r'__asm__\(""[^;]*;',';',source)
        helper=(ROOT/'tools/scripts/fixtures/views/func_8009424C.c').read_text()
        helper=re.sub(r'^#include.*\n','',helper,flags=re.M)
        prefix=r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,s)
#define RR_ASSERT_OFFSET(t,f,o)
#include "rr/draw_environment.h"
_Static_assert(sizeof(RrFrameEnvironment)==112,"env");
_Static_assert(offsetof(RrFrameEnvironment,rgb24)==24,"background");
_Static_assert(offsetof(RrFrameEnvironment,window)==12,"window");
_Static_assert(sizeof(RrGpuRectangle)==8,"rectangle");
_Static_assert(sizeof(unsigned long)>=sizeof(void*),"full native pointers");
_Static_assert(offsetof(RrFrameEnvironment,draw_packet1C)==28,"packet owner");
_Static_assert(offsetof(RrFrameEnvironment,clip)==0,"clip");
_Static_assert(offsetof(RrFrameEnvironment,offset_x)==8,"offset x");
_Static_assert(offsetof(RrFrameEnvironment,offset_y)==10,"offset y");
_Static_assert(offsetof(RrFrameEnvironment,page)==20,"page");
_Static_assert(offsetof(RrFrameEnvironment,dither)==22,"dither");
_Static_assert(offsetof(RrFrameEnvironment,draw_display)==23,"display flag");
_Static_assert(offsetof(RrFrameEnvironment,r)==25,"red");
_Static_assert(offsetof(RrFrameEnvironment,g)==26,"green");
_Static_assert(offsetof(RrFrameEnvironment,b)==27,"blue");
_Static_assert(offsetof(RrGpuRectangleFields,w)==4,"width");
_Static_assert(offsetof(RrGpuRectangleFields,h)==6,"height");
typedef struct {RrFrameEnvironment env;u32 packet[16];s16 width,height;u8 canary[20];} World;
typedef struct {int kind;int args[3];World snapshot;} Entry;
static World initial,actual,expected,*w;
static unsigned fast_cases,rectangle_cases;
static Entry entries[5],saved[5];static unsigned count,seed=0xF1EC3097;static int mutate,embedded;
static u32 *packet(World *p){return embedded?p->env.draw_packet1C:p->packet;}
u32 real_window(RrGpuRectangleFields *);
#define GpuVramWidth (w->width)
#define GpuVramHeight (w->height)
static u32 random32(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static u32 emit(int kind,int a,int b,int c){
 assert(count<5);Entry *p=&entries[count++];memset(p,0,sizeof(*p));p->kind=kind;p->args[0]=a;p->args[1]=b;p->args[2]=c;p->snapshot=*w;
 if(mutate){
  w->env.clip.x=(s16)(u16)((u16)w->env.clip.x+0x7123U);
  w->env.clip.y=(s16)(u16)((u16)w->env.clip.y+0x9397U);
  w->env.clip.w=(s16)(u16)((u16)w->env.clip.w+0xF234U);
  w->env.clip.h=(s16)(u16)((u16)w->env.clip.h+0x7821U);
  w->env.offset_x=(s16)(u16)((u16)w->env.offset_x+0x1234U);w->env.offset_y^=0x7fff;
  w->env.r+=73;w->env.g+=137;w->env.b+=219;w->env.page^=0x9ff;
  w->env.dither^=0xa1;w->env.draw_display^=0x3f;
  w->width=(s16)(u16)((u16)w->width+0x8123U);w->height=(s16)(u16)((u16)w->height+0x31U);
 }
 return 0xABCD0000U|((unsigned)kind*1297U)^((unsigned)a*17U)^((unsigned)b*213U)^((unsigned)c*319U);
}
u32 get_cs(s32 x,s32 y){return emit(0,x,y,0);}
u32 get_ce(s32 x,s32 y){return emit(1,x,y,0);}
u32 get_ofs(s32 x,s32 y){return emit(2,x,y,0);}
s32 get_mode(s32 a,s32 b,s32 c){return (s32)emit(3,a,b,c);}
u32 get_tw(RrGpuRectangleFields *p){assert(p==&w->env.window);u32 value=real_window(p);emit(4,12,0,0);return value;}
static int narrow(unsigned v){v&=65535;return v<32768?(int)v:(int)v-65536;}
static u16 read16(unsigned offset){u16 x;memcpy(&x,(u8*)&w->env+offset,2);return x;}
static unsigned read8(unsigned offset){return *((u8*)&w->env+offset);}
static void reference(void){
 u32 *out=packet(w);
 out[1]=emit(0,narrow(read16(0)),narrow(read16(2)),0);
 out[2]=emit(1,narrow(read16(0)+read16(4)-1U),narrow(read16(2)+read16(6)-1U),0);
 out[3]=emit(2,narrow(read16(8)),narrow(read16(10)),0);
 out[4]=emit(3,read8(23),read8(22),read16(20));
 u32 window=0xE2000000U+(read8(14)/8U)*32768U+(read8(12)/8U)*1024U+(((u32)(-narrow(read16(18)))%256U)/8U)*32U+(((u32)(-narrow(read16(16)))%256U)/8U);
 emit(4,12,0,0);out[5]=window;out[6]=0xE6000000U;
 unsigned length=6;
 if(read8(24)){
  int width=narrow(read16(4)),height=narrow(read16(6));
  if(width<0)width=0;else if(width>(int)w->width-1)width=(int)(u16)w->width-1;
  if(height<0)height=0;else if(height>(int)w->height-1)height=(int)(u16)w->height-1;
  unsigned x=read16(0),y=read16(2),command;
  if((x%64U)==0 && ((u16)width%64U)==0){
   command=0x02000000U;fast_cases++;
  }else{
   x=(u16)(x-read16(8));y=(u16)(y-read16(10));
   command=0x60000000U;rectangle_cases++;
  }
  out[7]=command|(read8(27)<<16)|(read8(26)<<8)|read8(25);
  out[8]=x|(y<<16);out[9]=(u16)width|((u32)(u16)height<<16);length=9;
 }
 ((u8*)out)[3]=(u8)length;
}
void SetDrawEnv2(u32 *,RrFrameEnvironment *);
int main(void){
 assert(real_window(NULL)==0);
 for(unsigned k=0;k<524288;k++){
  unsigned char *p=(unsigned char*)&initial;for(size_t j=0;j<sizeof(initial);j++)p[j]=(u8)random32();
  initial.env.clip.w=(s16)(u16)k;initial.env.clip.h=(s16)(u16)(k*31337U+0x1234U);
  initial.width=(s16)(u16)(k*17U);initial.height=(s16)(u16)(k*139U);
  initial.env.rgb24=((k>>16)&1)?(u8)(k|1U):0;mutate=(k>>17)&1;embedded=(k>>18)&1;
  if((k&3)==0)initial.env.clip.x=(s16)(u16)((random32()&0xFFC0U)-(mutate?5U*0x7123U:0));
  initial.env.window.w=(s16)(u16)k;initial.env.window.h=(s16)(u16)(k*31337U+0x3456U);
  actual=expected=initial;w=&actual;count=0;memset(entries,0,sizeof(entries));
  SetDrawEnv2(packet(w),&w->env);assert(count==5);memcpy(saved,entries,sizeof(saved));
  w=&expected;count=0;memset(entries,0,sizeof(entries));reference();
  assert(!memcmp(saved,entries,sizeof(saved)));assert(!memcmp(&actual,&expected,sizeof(actual)));
 }
 assert(fast_cases>1000 && rectangle_cases>1000);
 puts("524288 native fill-environment cases passed");
}
'''
        # Compile actual bodies; only empty scheduling hints/register constraints
        # are stripped. Both destinations are genuine u32 arrays in full owners.
        with TemporaryDirectory() as directory:
            c=Path(directory)/'draw.c'
            exe=Path(directory)/'draw'
            c.write_text(prefix+'\n#define get_tw real_window\n'+helper+'\n#undef get_tw\n'+source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined',
                            '-fno-sanitize-recover=all','-I',str(ROOT/'include'),
                            str(c),'-o',str(exe)],check=True,capture_output=True,text=True)
            result=subprocess.run([str(exe)],check=True,capture_output=True,text=True)
            self.assertIn('524288 native fill-environment cases passed',result.stdout)
