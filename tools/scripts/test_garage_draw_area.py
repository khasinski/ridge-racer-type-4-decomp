"""Draw-area packet with real resident constructor and coordinate helpers."""
from pathlib import Path
from tempfile import TemporaryDirectory
import re
import subprocess
import unittest

class GarageDrawAreaTest(unittest.TestCase):
    def test_geometry_shift_links_and_real_constructor(self):
        root=Path(__file__).resolve().parents[2]
        body=(root/'src/overlays/PAL/garage/func_8011ABB4.c').read_text()
        resident=(root/'tools/scripts/fixtures/legacy/gpu_coordinates.c').read_text()
        resident=re.sub(r' __asm__\("\$[0-9]+"\)','',resident)
        resident=re.sub(r'^.*__asm__.*\n','',resident,flags=re.M)
        constructor=re.sub(r'^extern[^\n]*;\n','',(root/'tools/scripts/fixtures/legacy/func_80093B14.c').read_text(),flags=re.M)
        source=r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4 || sizeof(t)==n,"PAL size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4 || offsetof(t,f)==n,"PAL offset")
#include "rr/garage_draw_area.h"
_Static_assert(sizeof(RrFrameStorage)==0x22778,"real frame stride");
_Static_assert(offsetof(RrFrameEnvironment,display.y)==0x5E,"comparison halfword");
RrFrameStorage FrameEnvs[2];static RrFrameStorage expected_frames[2];
RrGarageSelectionPrefix garage_logo_state;static RrGarageSelectionPrefix saved;
s32 DrawBufIndex;s16 GpuVramWidth,GpuVramHeight;
static struct Packets {u32 before[3],words[9],head,after[3];} actual,expected;
'''+resident+constructor+body+r'''
static u32 rng=0x1ABB4;
static u32 next(void){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng;}
static s32 word(u32 n){s32 v;memcpy(&v,&n,4);return v;}
static s16 half(u32 bits){u16 h=(u16)bits;s16 value;memcpy(&value,&h,2);return value;}
static s16 narrow(int64_t n){return half((u32)((uint64_t)n&65535U));}
static u32 command(s16 x,s16 y,u32 code){
 int64_t a=x,b=y;if(a<0)a=0;else if(a>=GpuVramWidth)a=GpuVramWidth-1;
 if(b<0)b=0;else if(b>=GpuVramHeight)b=GpuVramHeight-1;
 return code|((u32)a&1023U)|(((u32)b&1023U)*1024U);
}
static u32 *head(struct Packets *p,unsigned shape){
 static const unsigned index[]={3,4,5,0,6,2};return shape==0?&p->head:&p->words[index[shape-1]];
}
int main(void){unsigned i,shape,j;u32 x,y,w,h;uint64_t cases=0;
 static const s32 clocks[]={INT32_MIN,INT32_MAX,-1,0,1,0x00800000,-0x00800001,0x12345678};
 memset(&garage_logo_state,0,sizeof(garage_logo_state));
 for(i=0;i<65536;i++){
  s16 rx,ry;int64_t yy;
  garage_logo_state.halfC8=half(i);garage_logo_state.halfCA=half(i*73U+17);
  memcpy(&saved,&garage_logo_state,sizeof(saved));
  FrameEnvs[0].environment.display.y=half(i);
  FrameEnvs[1].environment.display.y=half(i+(i&1));
  expected_frames[0].environment.display.y=FrameEnvs[0].environment.display.y;
  expected_frames[1].environment.display.y=FrameEnvs[1].environment.display.y;
  DrawBufIndex=i%9==8?word(next()):clocks[i%8];
  GpuVramWidth=i%3==0?1:i%3==1?320:1024;GpuVramHeight=i%3==0?512:i%3==1?256:1;
  x=next();y=next();w=(u16)i;h=(u16)(i*73U+17);
  rx=narrow((int64_t)saved.halfC8+x);ry=narrow((int64_t)saved.halfCA+y);
  yy=ry;if(i&1)yy+=(int64_t)DrawBufIndex*256;ry=narrow(yy);
  for(shape=0;shape<7;shape++){
   u32 *ordering,*eorder,*pkt=actual.words+3;u8 *result;
   for(j=0;j<3;j++){actual.before[j]=next();actual.after[j]=next();}
   for(j=0;j<9;j++)actual.words[j]=next();actual.head=next();
   memcpy(&expected,&actual,sizeof(actual));ordering=head(&actual,shape);eorder=head(&expected,shape);
   expected.words[3]=(expected.words[3]&0x00FFFFFFU)|0x02000000U;
   expected.words[4]=command(rx,ry,0xE3000000U);
   expected.words[5]=command(narrow((int64_t)(u16)rx+w-1),narrow((int64_t)(u16)ry+h-1),0xE4000000U);
   expected.words[3]=(expected.words[3]&0xFF000000U)|(*eorder&0xFFFFFFU);
   *eorder=(*eorder&0xFF000000U)|((u32)((uintptr_t)pkt&0xFFFFFFU));
   result=garage_add_draw_area(ordering,pkt,x,y,(u16)w,(u16)h);
   assert(result==(u8*)(actual.words+6));assert((uintptr_t)result>UINT32_MAX);
   assert(memcmp(&actual,&expected,sizeof(actual))==0);
   assert(memcmp(&saved,&garage_logo_state,sizeof(saved))==0);cases++;
  }
 }
 assert(memcmp(FrameEnvs,expected_frames,sizeof(expected_frames))==0);
 assert(cases==458752ULL);return 0;
}
'''
        with TemporaryDirectory() as tmp:
            p=Path(tmp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
