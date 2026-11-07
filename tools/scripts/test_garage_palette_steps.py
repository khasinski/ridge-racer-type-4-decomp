"""RGB editing with genuine palette unions and actual backup/clip/hit bodies."""
from pathlib import Path
from tempfile import TemporaryDirectory
import re
import subprocess
import unittest

class Overlay662PaletteStepsTest(unittest.TestCase):
    def test_all_palette_words_gates_and_fresh_captured_entry(self):
        root=Path(__file__).resolve().parents[2]
        paths=['8011E6C8','8011E7C0','8011E7DC','8011D120','8012349C','801235F4','80123750','801238AC','80123A0C','80123B68']
        bodies='\n'.join(re.sub(r'^extern[^\n]*;\n','',(root/f'src/overlays/PAL/garage/func_{a}.c').read_text(),flags=re.M) for a in paths)
        bodies+='\n'+re.sub(r'^extern[^\n]*;\n','',(root/'tools/scripts/fixtures/views/func_8004F218.c').read_text(),flags=re.M)+'\n'+re.sub(r'^extern[^\n]*;\n','',(root/'tools/scripts/fixtures/views/func_8004F24C.c').read_text(),flags=re.M)
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
#include "rr/overlay662_palette_edit.h"
#include "rr/overlay662_tool_action.h"
#include "rr/audio_pan.h"
static struct World {u32 head[4];RrGarageSelectionPrefix garage;RrGarageWorkingStorage work;u32 image[4][512];RrPalette16 palette[2];RrGarageButton buttons[46];RrGarageRectangle *table[46];u32 tail[4];} world,expected,checkpoint;
#define garage_D_801276A8 world.garage
#define garage_D_80127668 world.work
#define garage_D_801273B8 world.table
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
static unsigned family,bits,flags,slot,captured_pool,color_index,expected_sound,calls;
static s32 program;static uint64_t cases;
static unsigned button_index(RrGarageRectangle*p){unsigned n=(unsigned)((RrGarageButton*)p-world.buttons);assert(n<46);return n;}
static void sound_effect(struct World*w){w->palette[captured_pool].entries[color_index]=(u16)(bits^0xA55AU);w->garage.active21^=1;w->garage.config22=(u8)(color_index^15);w->garage.palette48[!w->garage.active21]=world.palette[!captured_pool].words;w->table[slot]=&world.buttons[(bits+19)%46].rectangle;w->work.words[1]^=0xA551FF45U;w->garage.reset08[0]=half(bits);}
void TriggerSfx(s32 p,s32 level,s32 pan,s32 pitch,s32 voice){assert(expected_sound&&calls++==0);assert(p==program&&level==127&&pan==64&&pitch==0x1E00&&voice==-1);assert(memcmp(&world,&checkpoint,sizeof(world))==0);sound_effect(&world);}
static void reference(void){unsigned n,j,shift=5*(family/2);int value=(bits>>shift)&31;
 if(flags&1){if(flags&2){expected.work.words[0]&=~8U;clip(13,63,294,346);return;}}
 else{if(!(flags&16)||!(flags&4))return;expected.work.words[4]=slot;expected.work.words[0]|=8;n=button_index(expected.table[slot]);for(j=0;j<4;j++)expected.work.halves[24+j]=half(expected.buttons[n].rectangle[j]);backup();}
 if(!(flags&8))return;
 value+=(family&1)?1:-1;program=0;if(value<0){value=0;program=4;}if(value>31){value=31;program=4;}
 n=button_index(expected.table[slot]);expected.buttons[n].timer0E=7;expected_sound=1;checkpoint=expected;sound_effect(&expected);
 expected.palette[captured_pool].entries[color_index]=(u16)((expected.palette[captured_pool].entries[color_index]&~(31U<<shift))|((unsigned)value<<shift));
}
static void run(void){unsigned j,k,n;int x=(family&1)?213:141,y=355+(int)(family/2)*18;
 slot=40+family;memset(&world,0,sizeof(world));for(j=0;j<4;j++){world.head[j]=next();world.tail[j]=next();for(k=0;k<512;k++)world.image[j][k]=bits^(j*0x12345678U)^k;}
 for(j=0;j<2;j++){for(k=0;k<8;k++)world.palette[j].words[k]=next();world.garage.image38[j]=world.image[j];world.garage.image40[j]=world.image[j+2];world.garage.palette48[j]=world.palette[j].words;}
 if(bits%11==0){world.garage.image38[1]=world.garage.image38[0];world.garage.image40[1]=world.garage.image40[0];world.garage.palette48[1]=world.garage.palette48[0];}
 world.garage.active21=bits&1;world.garage.config22=(bits>>1)&15;world.garage.dirtyD3=(u8)next();captured_pool=palette_index(world.garage.palette48[world.garage.active21]);color_index=world.garage.config22;world.palette[captured_pool].entries[color_index]=(u16)bits;
 for(j=0;j<16;j++)world.work.words[j]=next();
 for(j=0;j<46;j++){world.table[j]=&world.buttons[j].rectangle;world.buttons[j].key0C=(s16)j;world.buttons[j].timer0E=half(bits+j);for(k=0;k<4;k++)world.buttons[j].rectangle[k]=(u16)next();}
 n=bits%46;world.table[slot]=&world.buttons[n].rectangle;
 world.work.words[0]=0xA5510000U|((flags&1)?8:0);world.work.words[1]=0xAAE00000U|((flags&2)?4:0)|((flags&4)?1:0)|((flags&8)?0x40U:0);
 world.work.points.cursor14=(RrGaragePoint){(s16)(x+((flags&16)?0:8)),(s16)y};
 expected_sound=calls=0;expected=world;reference();switch(family){
 case 0:garage_func_8012349C();break;case 1:garage_func_801235F4();break;case 2:garage_func_80123750();break;case 3:garage_func_801238AC();break;case 4:garage_func_80123A0C();break;case 5:garage_func_80123B68();break;}
 assert(calls==expected_sound);assert(memcmp(&world,&expected,sizeof(world))==0);assert((uintptr_t)world.palette>UINT32_MAX);cases++;
}
int main(void){unsigned path;
 for(bits=0;bits<65536;bits++)for(family=0;family<6;family++)for(path=0;path<2;path++){flags=path?28:9;run();}
 for(bits=0;bits<256;bits++)for(family=0;family<6;family++)for(flags=0;flags<32;flags++)run();
 assert(cases==835584ULL);return 0;}
'''
        with TemporaryDirectory() as tmp:
            p=Path(tmp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
