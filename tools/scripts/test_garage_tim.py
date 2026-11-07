"""TIM export: genuine complete owners, unaligned byte sources and live image aliases."""
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess
import unittest

class Overlay662TimTest(unittest.TestCase):
    def test_headers_full_copy_palette_and_live_compositor_aliases(self):
        root=Path(__file__).resolve().parents[2]
        bodies='\n'.join((root/f'src/overlays/PAL/garage/func_{a}.c').read_text() for a in ['80125F60','80126044'])
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
#include "rr/overlay662_tim.h"
typedef union {RrGarageTimImage tim;struct{u8 prefix;RrGarageTimBytes file;}at1;struct{u8 prefix[2];RrGarageTimBytes file;}at2;struct{u8 prefix[3];RrGarageTimBytes file;}at3;} SourceViews;
static struct World{u32 head[4];RrGarageSelectionPrefix garage;RrGarageTimImage output;SourceViews source[2];RrPalette16 palette[2];u32 images[2][512];u32 tail[4];}world,expected;
#define garage_logo_state world.garage
#define SaveDataBackup world.output
'''+bodies+r'''
static u32 rng=0x26044;static u32 next(void){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng;}
static size_t offset(const void*p){uintptr_t at=(uintptr_t)p,start=(uintptr_t)&world;assert(at>=start&&at<start+sizeof(world));return(size_t)(at-start);}
static void reference(void){unsigned active=expected.garage.active21,i,n;size_t src=offset(expected.garage.tim2C[active]),image=offset(expected.garage.image38[active]),pal=offset(expected.garage.palette48[active]);u8*bytes=(u8*)&expected;
 memmove(expected.output.file.data,bytes+src,sizeof(expected.output.file));
 expected.output.words[0]=16;expected.output.words[1]=8;expected.output.words[2]=44;expected.output.words[3]=0x00400280U;expected.output.words[4]=0x00010010U;
 memmove(expected.output.layout.palette.raw.data,bytes+pal,32);
 expected.output.words[13]=0x80C;expected.output.words[14]=0x280;expected.output.words[15]=0x00400010;
 for(i=0;i<512;i++){u32 value,mask=0;memcpy(&value,bytes+image+4*i,4);
  for(n=0;n<8;n++)if(((value>>(4*n))&15U)==0)mask|=15U<<(4*n);
  expected.output.words[16+i]&=mask;memcpy(&value,bytes+image+4*i,4);expected.output.words[16+i]|=value;
 }
}
int main(void){unsigned scenario,j,k,active,alignment;for(scenario=0;scenario<65536;scenario++){
 memset(&world,0,sizeof(world));for(j=0;j<4;j++){world.head[j]=next();world.tail[j]=next();}
 for(j=0;j<sizeof(world.garage);j++)((u8*)&world.garage)[j]=(u8)next();
 for(j=0;j<sizeof(world.source);j++)((u8*)world.source)[j]=(u8)next();
 for(j=0;j<528;j++)world.output.words[j]=next();
 alignment=(scenario>>1)&3;active=scenario&1;world.garage.active21=(u8)active;
 for(j=0;j<2;j++){for(k=0;k<512;k++)world.images[j][k]=next();for(k=0;k<8;k++)world.palette[j].words[k]=next();
  world.garage.buffers24[j]=world.images[j];world.garage.image38[j]=world.images[j];world.garage.image40[j]=world.images[!j];world.garage.palette48[j]=world.palette[j].words;
  switch(alignment){case 0:world.garage.tim2C[j]=&world.source[j].tim.file;break;case 1:world.garage.tim2C[j]=&world.source[j].at1.file;break;case 2:world.garage.tim2C[j]=&world.source[j].at2.file;break;default:world.garage.tim2C[j]=&world.source[j].at3.file;break;}
  assert(((uintptr_t)world.garage.tim2C[j]&3)==alignment);
 }
 world.garage.scratch34=world.images[0];
 if(scenario%13==0)world.garage.tim2C[active]=&world.output.file;
 if(scenario&8)world.garage.palette48[active]=world.output.layout.palette.words;
 switch((scenario>>4)%3){case 1:world.garage.image38[active]=world.output.words+(scenario%17);break;case 2:world.garage.image38[active]=world.source[active].tim.layout.pixels;break;default:break;}
 expected=world;reference();garage_build_save_tim();assert(memcmp(&world,&expected,sizeof(world))==0);
 assert((uintptr_t)world.garage.image38[active]>UINT32_MAX);}
 return 0;}
'''
        with TemporaryDirectory() as tmp:
            p=Path(tmp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
