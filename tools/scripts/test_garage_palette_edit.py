"""RGB editing with genuine palette unions and actual backup/clip/hit bodies."""
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess
import unittest

class GaragePaletteEditTest(unittest.TestCase):
    def test_all_cursor_halfwords_start_gates_and_backup(self):
        root=Path(__file__).resolve().parents[2]
        paths=['8011E6C8','8011E7C0','8011D120','80124258','801243DC','80124560']
        bodies='\n'.join((root/f'src/overlays/PAL/garage/func_{a}.c').read_text() for a in paths)
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
#include "rr/garage_palette_edit.h"
static struct World {u32 head[4];RrGarageSelectionPrefix garage;RrGarageWorkingStorage work;u32 image[4][512];RrPalette16 palette[2];s32 changed[3];u16 original[3];u32 tail[4];} world,expected;
#define garage_logo_state world.garage
#define garage_logo_work world.work
#define garage_red_slider_backed_up world.changed[0]
#define garage_green_slider_backed_up world.changed[1]
#define garage_blue_slider_backed_up world.changed[2]
#define garage_red_slider_start world.original[0]
#define garage_green_slider_start world.original[1]
#define garage_blue_slider_start world.original[2]
'''+bodies+r'''
static u32 rng=0x24258;static u32 next(void){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng;}
static s16 half(u32 x){u16 n=(u16)x;s16 v;memcpy(&v,&n,2);return v;}
static unsigned image_index(u32*p){unsigned n;for(n=0;n<4;n++)if(p==world.image[n])return n;assert(0);return 0;}
static unsigned palette_index(u32*p){unsigned n;for(n=0;n<2;n++)if(p==world.palette[n].words)return n;assert(0);return 0;}
static void clip(s16 x,s16 y,s16 w,s16 h){expected.work.halves[24]=x;expected.work.halves[25]=y;expected.work.halves[26]=w;expected.work.halves[27]=h;}
static void backup(void){unsigned a=expected.garage.active21,b=!a,j,s,d;expected.garage.dirtyD3=1;
 s=image_index(expected.garage.image38[a]);d=image_index(expected.garage.image38[b]);for(j=0;j<512;j++)expected.image[d][j]=expected.image[s][j];
 s=image_index(expected.garage.image40[a]);d=image_index(expected.garage.image40[b]);for(j=0;j<512;j++)expected.image[d][j]=expected.image[s][j];
 s=palette_index(expected.garage.palette48[a]);d=palette_index(expected.garage.palette48[b]);for(j=0;j<8;j++)expected.palette[d].words[j]=expected.palette[s].words[j];}
static void reference(unsigned family,unsigned kind){unsigned p,index=expected.garage.config22,shift=family*5;int delta;u16 value,old,mask;
 if(kind==2){expected.work.words[0]&=~8U;clip(13,63,294,346);return;}if(kind>=3)return;
 p=palette_index(expected.garage.palette48[expected.garage.active21]);
 if(kind==1){expected.work.words[4]=48+family;expected.work.words[0]|=8;clip(149,(s16)(355+18*family),64,12);expected.changed[family]=0;expected.original[family]=(expected.palette[p].entries[index]>>shift)&31;}
 delta=expected.work.points.cursor14.x-149;value=(u16)(delta<0?-((-delta)/2):delta/2);
 if(value!=expected.original[family]&&expected.changed[family]==0){expected.changed[family]=1;backup();}
 p=palette_index(expected.garage.palette48[expected.garage.active21]);old=expected.palette[p].entries[index];mask=(u16)~(31U<<shift);expected.palette[p].entries[index]=(u16)((old&mask)|((u32)value<<shift));
}
int main(void){unsigned bits,family,kind,j,k;uint64_t cases=0;
 for(bits=0;bits<65536;bits++)for(family=0;family<3;family++)for(kind=0;kind<5;kind++){
 memset(&world,0,sizeof(world));for(j=0;j<4;j++){world.head[j]=next();world.tail[j]=next();for(k=0;k<512;k++)world.image[j][k]=next();}
 for(j=0;j<2;j++){for(k=0;k<8;k++)world.palette[j].words[k]=next();world.garage.image38[j]=world.image[j];world.garage.image40[j]=world.image[j+2];world.garage.palette48[j]=world.palette[j].words;}
 if(bits%11==0){world.garage.image38[1]=world.garage.image38[0];world.garage.image40[1]=world.garage.image40[0];world.garage.palette48[1]=world.garage.palette48[0];}
 world.garage.active21=bits&1;world.garage.config22=(bits>>1)&15;world.garage.dirtyD3=(u8)next();
 for(j=0;j<16;j++)world.work.words[j]=next();for(j=0;j<3;j++){world.changed[j]=(bits&4)?0:(s32)((bits&8)?1:-1);world.original[j]=(u16)next();}
 world.original[family]=(u16)((bits>>3)&31);world.work.words[0]=0xA5510000U|((kind==0||kind==2)?8:0);world.work.words[1]=0xAAE00000U|(kind==2?4:kind==4?0:1);
 world.work.points.cursor14.x=kind==0?half(bits):kind==3?214:(s16)(149+bits%65);world.work.points.cursor14.y=(s16)(355+family*18);
 if(kind>=2){world.garage.active21=255;world.garage.config22=255;for(j=0;j<2;j++){world.garage.image38[j]=0;world.garage.image40[j]=0;world.garage.palette48[j]=0;}}
 expected=world;reference(family,kind);if(family==0)garage_palette_red_slider();else if(family==1)garage_palette_green_slider();else garage_palette_blue_slider();
 assert(memcmp(&world,&expected,sizeof(world))==0);assert((uintptr_t)world.palette>UINT32_MAX);cases++;
 }assert(cases==983040ULL);return 0;}
'''
        with TemporaryDirectory() as tmp:
            p=Path(tmp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
