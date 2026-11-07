"""Actual two-bank initialization plus its ready draw/display helpers, UBSan."""
from pathlib import Path
import re,subprocess,tempfile,unittest
root=Path(__file__).resolve().parents[2]
src=(root/'tools/scripts/fixtures/views/func_8001ECE8.c').read_text();src=re.sub(r'^#include[^\n]*\n|extern[^;]*;','',src,flags=re.M)
src=re.sub(r' asm\("\$\d+"\)','',src);src=re.sub(r'__asm__\("".*?\);','',src)
draw=(root/'tools/scripts/fixtures/views/func_800928E4.c').read_text().replace('SetDefDrawEnv','native_draw')
disp=(root/'tools/scripts/fixtures/views/func_80092998.c').read_text().replace('SetDefDispEnv','native_display')
hook=(root/'tools/scripts/fixtures/views/func_8001ECE0.c').read_text();hook=re.sub(r'^#include[^\n]*\n','',hook,flags=re.M).replace('SetupDrawDispEnvsHookStub','native_bank_hook')
fixture=r'''
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "types.h"
#include <stddef.h>
#include "rr/layout.h"
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_OFFSET(type,field,offset) _Static_assert(offsetof(type,field)==offset,"native layout")
#include "rr/frame_loop.h"

static RrFrameStorage actual[2],expected[2];
#define FrameEnvs actual
#define SecondaryFrameEnv actual[1].environment
#define SecondaryFrameDispEnv actual[1].environment.display
RrRuntimeSettings Options;
static int step,video;
static u32 input_flags;
static int GetVideoMode(void) {return video;}
''' + draw+disp+hook+r'''
void SetGeomOffset(s32 x,s32 y) {assert(step++==0 && x==160 && y==(s32)((((input_flags>>1)&1)+1)*128));}
void SetGeomScreen(s32 z) {assert(step++==1 && z==290);}
void *SetDefDrawEnv(void *e,s32 x,s32 y,s32 w,s32 h) {
 int index=step-2;assert(index>=0 && index<2);step++;
 assert(e==&actual[index]);assert(x==0 && y==(index?(s32)((1-((input_flags>>1)&1))*256):0));
 assert(w==((input_flags&4)?480:320) && h==(s32)((((input_flags>>1)&1)+1)*256));
 return native_draw(e,x,y,w,h);
}
void *SetDefDispEnv(void *e,s32 x,s32 y,s32 w,s32 h) {
 int index=step-4;assert(index>=0 && index<2);step++;
 assert(e==&actual[index].environment.display);assert(x==0 && y==(index?0:(s32)((1-((input_flags>>1)&1))*256)));
 assert(w==((input_flags&4)?480:320) && h==(s32)((((input_flags>>1)&1)+1)*256));
 return native_display(e,x,y,w,h);
}
void SetupDrawDispEnvsHookStub(s32 bank) {assert(step++==6+bank && bank>=0 && bank<2);native_bank_hook(bank);}
''' + src + r'''
static void reference(s32 r,s32 g,s16 b,u32 flags) {
 int i;int interlace=(flags>>1)&1,height=(interlace+1)*256,width=(flags&4)?480:320;
 for(i=0;i<2;i++) {
  RrFrameEnvironment *e=&expected[i].environment;
  e->clip.x=0;e->clip.y=i?(1-interlace)*256:0;e->clip.w=width;e->clip.h=height;
  e->offset_x=e->clip.x;e->offset_y=e->clip.y;
  e->page=10;e->dither=1;e->draw_display=video?(height<289):(height<257);
  e->rgb24=flags&1;e->r=(u8)r;e->g=(u8)g;e->b=(u8)b;
  e->window.x=e->window.y=e->window.w=e->window.h=0;
  e->display.x=0;e->display.y=i?0:(1-interlace)*256;e->display.w=320;e->display.h=height;
  e->screen.x=Options.halves[26];e->screen.y=Options.halves[28];e->screen.w=0;e->screen.h=256;
  e->interlace=interlace;e->high_res=(flags&4)?1:0;e->pad6E[0]=e->pad6E[1]=0;
 }
}
static void run(s32 r,s32 g,s16 b,u32 flags,int mode,u16 sx,u16 sy) {
 memset(actual,0xA7,sizeof(actual));memcpy(expected,actual,sizeof(actual));
 input_flags=flags;step=0;video=mode;Options.halves[26]=sx;Options.halves[28]=sy;
 reference(r,g,b,flags);SetupDrawDispEnvs(r,g,b,flags);
 assert(step==8 && !memcmp(actual,expected,sizeof(actual)));
}
int main(void) {
 u32 flags[]={0,1,2,3,4,5,6,7,0xFFFFFFFFU,0x80000000U,0x7FFFFFFFU,0xFFFFFFF8U};
 s32 colors[]={INT32_MIN,-1,0,1,127,128,255,256,INT32_MAX};
 int i,j,k;u32 seed=0x12345678;
 assert(sizeof(RrFrameEnvironment)==0x70 && sizeof(RrFrameStorage)==0x22778);
 for(i=0;i<12;i++)for(j=0;j<9;j++)for(k=0;k<2;k++)run(colors[j],colors[8-j],(s16)colors[j],flags[i],k,0x8000,0xFFFF);
 for(i=0;i<2000;i++){seed=seed*1664525U+1013904223U;run((s32)seed,(s32)(seed^0x55555555U),(s16)(seed>>16),seed,(seed>>3)&1,(u16)seed,(u16)(seed>>16));}
 return 0;
}
'''
class FrameEnvironmentTests(unittest.TestCase):
    def test_actual_initialization_with_real_helpers(self):
        with tempfile.TemporaryDirectory() as t:
            c=Path(t)/'env.c';exe=Path(t)/'env';c.write_text(fixture)
            subprocess.run(['cc','-std=gnu99','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(c),'-o',str(exe)],check=True,capture_output=True)
            subprocess.run([str(exe)],check=True,capture_output=True)
if __name__=='__main__':unittest.main()
