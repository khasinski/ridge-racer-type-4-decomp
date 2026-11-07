"""Pixel color picker with real hit testing and actual sound-wrapper closure."""
from pathlib import Path
from tempfile import TemporaryDirectory
import re
import subprocess
import unittest

class Overlay662ColorPickTest(unittest.TestCase):
    def test_all_pixels_color_pairs_zoom_and_gates(self):
        root=Path(__file__).resolve().parents[2]
        paths=['src/overlays/PAL/garage/func_8011E6C8.c','tools/scripts/fixtures/views/func_8004F218.c','tools/scripts/fixtures/views/func_8004F24C.c','src/overlays/PAL/garage/func_80125370.c']
        bodies='\n'.join(re.sub(r'^extern[^\n]*;\n','',(root/p).read_text(),flags=re.M) for p in paths)
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
#include "rr/overlay662_color_pick.h"
#include "rr/audio_pan.h"
RrGarageSelectionPrefix garage_D_801276A8;RrGarageWorkingStorage garage_D_80127668;
static RrGarageSelectionPrefix expected;static RrGarageWorkingStorage expected_work;
static struct Image {u32 before;RrGarageImage512 words;u32 after;} images[4],image_model[4];
static unsigned audio_calls,expected_mode,expected_before,after_color;
void TriggerSfx(s32 program,s32 level,s32 pan,s32 pitch,s32 voice){
 assert((unsigned)program==expected_mode&&level==127&&pan==64&&pitch==0x1E00&&voice==-1);
 assert(garage_D_801276A8.config22==expected_before);audio_calls++;
 garage_D_801276A8.config22=(u8)after_color;garage_D_801276A8.config1C=0;garage_D_801276A8.active21=255;
 garage_D_801276A8.reset08[0]=INT16_MIN;garage_D_801276A8.image38[0]=0;
 garage_D_80127668.words[1]^=0x12345678U;
}
'''+bodies+r'''
static unsigned origin(unsigned pixel,unsigned crop){return pixel<crop/2?0:pixel-crop/2>64-crop?64-crop:pixel-crop/2;}
static void setpixel(struct Image *p,unsigned x,unsigned y,unsigned color){
 u8 *bytes=(u8*)&p->words;unsigned at=y*32+x/2;
 bytes[at]=x%2?(u8)((color*16)|11):(u8)(0xA0|color);
}
int main(void){unsigned pixel,pair,active,mode,j;uint64_t cases=0;
 static const unsigned zooms[]={2,4,8,16};
 static const s16 outside[8][2]={{81,71},{210,71},{82,70},{82,327},{INT16_MIN,71},{INT16_MAX,71},{82,INT16_MIN},{82,INT16_MAX}};
 for(j=0;j<4;j++){images[j].before=0xFACE1234U+j;images[j].after=0xDEAD5678U+j;memset(images[j].words,(int)(0x21+j*17),2048);}
 memcpy(image_model,images,sizeof(images));
 for(mode=0;mode<10;mode++){
  RrGarageSelectionPrefix before;RrGarageWorkingStorage work;
  memset(&garage_D_801276A8,0,sizeof(garage_D_801276A8));memset(&garage_D_80127668,0,sizeof(garage_D_80127668));
  garage_D_801276A8.active21=255;garage_D_801276A8.config1C=0;
  garage_D_80127668.words[1]=mode<8?2:mode==8?0:1;
  garage_D_80127668.halves[10]=mode<8?outside[mode][0]:82;
  garage_D_80127668.halves[11]=mode<8?outside[mode][1]:71;
  memcpy(&before,&garage_D_801276A8,sizeof(before));memcpy(&work,&garage_D_80127668,sizeof(work));
  garage_func_80125370();assert(audio_calls==0);
  assert(memcmp(&before,&garage_D_801276A8,sizeof(before))==0&&memcmp(&work,&garage_D_80127668,sizeof(work))==0);cases++;
 }
 for(pixel=0;pixel<4096;pixel++)for(pair=0;pair<256;pair++)for(active=0;active<2;active++){
  unsigned x=pixel%64,y=pixel/64,zoom=zooms[(pair>>4)&3],crop=128/zoom;
  unsigned ox=origin(x,crop),oy=origin(y,crop),first=pair%16,second=pair/16,selected=first?first:second;
  unsigned b0=active,b1=2+active,at=y*32+x/2;
  unsigned old=(pixel+pair+active)&255,before_calls=audio_calls;
  memset(&garage_D_801276A8,0,sizeof(garage_D_801276A8));memset(&garage_D_80127668,0,sizeof(garage_D_80127668));
  for(j=0;j<2;j++){garage_D_801276A8.image38[j]=images[j].words;garage_D_801276A8.image40[j]=images[2+j].words;}
  if(pair%17==0){b1=b0;garage_D_801276A8.image40[active]=images[b0].words;}
  setpixel(&images[b0],x,y,first);setpixel(&image_model[b0],x,y,first);
  setpixel(&images[b1],x,y,second);setpixel(&image_model[b1],x,y,second);
  garage_D_801276A8.active21=(u8)active;garage_D_801276A8.config1C=(u8)zoom;garage_D_801276A8.config22=(u8)old;
  garage_D_801276A8.reset08[0]=(s16)ox;garage_D_801276A8.reset08[1]=(s16)oy;
  garage_D_80127668.words[1]=2;
  garage_D_80127668.halves[10]=(s16)(82+(x-ox)*zoom+(pair+active)%zoom);
  garage_D_80127668.halves[11]=(s16)(71+(y-oy)*zoom*2+(pair+active)%(zoom*2));
  assert(garage_D_80127668.halves[10]<210&&garage_D_80127668.halves[11]<327);
  expected_mode=selected?1:4;expected_before=selected?selected:old;after_color=(pair^pixel^0x9D)&255;
  memcpy(&expected,&garage_D_801276A8,sizeof(expected));memcpy(&expected_work,&garage_D_80127668,sizeof(expected_work));
  expected.config22=(u8)after_color;expected.config1C=0;expected.active21=255;expected.reset08[0]=INT16_MIN;expected.image38[0]=0;
  expected_work.words[1]^=0x12345678U;
  garage_func_80125370();assert(audio_calls==before_calls+1);
  assert(memcmp(&expected,&garage_D_801276A8,sizeof(expected))==0);
  assert(memcmp(&expected_work,&garage_D_80127668,sizeof(expected_work))==0);
  assert(((u8*)&images[b0].words)[at]==((u8*)&image_model[b0].words)[at]);
  assert(((u8*)&images[b1].words)[at]==((u8*)&image_model[b1].words)[at]);
  assert((uintptr_t)images[b0].words>UINT32_MAX);cases++;
 }
 assert(memcmp(images,image_model,sizeof(images))==0);assert(audio_calls==2097152U);assert(cases==2097162ULL);return 0;
}
'''
        with TemporaryDirectory() as tmp:
            p=Path(tmp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
