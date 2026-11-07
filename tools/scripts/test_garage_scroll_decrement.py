"""Decrement scroll actions: real hit/clip/snapshot/sound-wrapper bodies, typed audio boundary."""
from pathlib import Path
from tempfile import TemporaryDirectory
import re
import struct
import subprocess
import unittest

class Overlay662ScrollDecrementTest(unittest.TestCase):
    def test_all_halfwords_and_input_gates(self):
        root=Path(__file__).resolve().parents[2]
        blob=(root/'disc/PAL/R4.BIN').read_bytes();sector=struct.unpack_from('<I',blob,4+662*4)[0]
        rows=[struct.unpack_from('<8h',blob,sector*2048+0x80126F88-0x80114878+16*i) for i in range(46)]
        self.assertEqual(sorted(r[6] for r in rows),list(range(46)))
        data='static const RrGarageButton original[46]={\n'+',\n'.join('{{'+','.join(map(str,r[:4]))+'},'+','.join(map(str,r[4:]))+'}' for r in rows)+'\n};\n'
        paths=['src/overlays/PAL/garage/func_'+a+'.c' for a in ['8011E6C8','8011E7C0','8011E7DC','80122FB4','80123228']]+['tools/scripts/fixtures/views/func_8004F218.c','tools/scripts/fixtures/views/func_8004F24C.c']
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
#include "rr/overlay662_scroll_actions.h"
#include "rr/audio_pan.h"
static struct World {u32 guard[4];RrGarageSelectionPrefix garage;RrGarageWorkingStorage work;RrGarageButton buttons[46];RrGarageRectangle *table[46];u32 tail[4];} world,expected,checkpoint;
#define garage_D_801276A8 world.garage
#define garage_D_80127668 world.work
#define garage_D_801273B8 world.table
'''+data+bodies+r'''
static unsigned axis,slot,bits,path,expected_sound,calls;static s32 sound_program;static uint64_t cases;
static u32 rng=0x22FB4;static u32 next(void){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng;}
static s16 half(u32 x){u16 n=(u16)x;s16 v;memcpy(&v,&n,2);return v;}
static void sound_effect(struct World *w){w->garage.reset08[axis]=half(bits^0x91A5U);w->garage.reset08[!axis]=half(bits^0x5623U);w->work.words[0]^=0xFE234567U;w->work.words[1]^=0x76543210U;w->table[slot]=&world.buttons[(bits+17)%46].rectangle;w->garage.config22=(u8)bits;}
void TriggerSfx(s32 program,s32 level,s32 pan,s32 pitch,s32 voice){assert(expected_sound&&calls++==0);assert(program==sound_program&&level==127&&pan==64&&pitch==0x1E00&&voice==-1);assert(memcmp(&world,&checkpoint,sizeof(world))==0);sound_effect(&world);}
static unsigned button_index(RrGarageRectangle *p){unsigned n=(unsigned)((RrGarageButton*)p-world.buttons);assert(n<46);return n;}
static void clip(s16 x,s16 y,s16 w,s16 h){expected.work.halves[24]=x;expected.work.halves[25]=y;expected.work.halves[26]=w;expected.work.halves[27]=h;}
static void reference(void){unsigned n,j;int active=(path&1)!=0,cancel=(path&2)!=0,press=(path&4)!=0,repeat=(path&8)!=0,inside=(path&16)!=0;
 if(active&&cancel){expected.work.words[0]&=~8U;clip(13,63,294,346);return;}
 if(!active){if(!inside||!press)return;expected.work.words[4]=slot;expected.work.words[0]|=8U;n=button_index(expected.table[slot]);for(j=0;j<4;j++)expected.work.halves[24+j]=half(expected.buttons[n].rectangle[j]);}
 if(!repeat)return;
 n=button_index(expected.table[slot]);expected.buttons[n].timer0E=7;
 expected.garage.reset08[axis]=half(bits-1U);
 if(expected.garage.reset08[axis]<0){expected.garage.reset08[axis]=0;sound_program=4;}else sound_program=0;
 expected_sound=1;memcpy(&checkpoint,&expected,sizeof(expected));sound_effect(&expected);
}
int main(void){unsigned j,n;for(bits=0;bits<65536;bits++)for(axis=0;axis<2;axis++)for(path=0;path<32;path++){
 slot=axis?36:38;memset(&world,0,sizeof(world));memcpy(world.buttons,original,sizeof(original));
 for(j=0;j<4;j++){world.guard[j]=next();world.tail[j]=next();world.garage.selectorsB8[j]=(s32)j;}
 for(j=0;j<10;j++)world.garage.reset08[j]=half(next());world.garage.reset08[axis]=half(bits);
 for(j=0;j<16;j++)world.work.words[j]=next();
 for(j=0;j<46;j++){world.table[(unsigned)world.buttons[j].key0C]=&world.buttons[j].rectangle;world.buttons[j].timer0E=half(next());}
 n=bits%46;world.table[slot]=&world.buttons[n].rectangle;
 world.work.words[0]=(next()&~8U)|((path&1)?8U:0U);
 world.work.words[1]=(next()&~0x45U)|((path&2)?4U:0U)|((path&4)?1U:0U)|((path&8)?0x40U:0U);
 world.work.points.cursor14=(RrGaragePoint){axis?((path&16)?215:221):((path&16)?81:90),axis?69:335};
 expected_sound=calls=0;memcpy(&expected,&world,sizeof(world));reference();if(axis)garage_func_80122FB4();else garage_func_80123228();
 assert(calls==expected_sound);assert(memcmp(&world,&expected,sizeof(world))==0);assert((uintptr_t)world.table[slot]>UINT32_MAX);cases++;
 }assert(cases==4194304ULL);return 0;}
'''
        with TemporaryDirectory() as tmp:
            p=Path(tmp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
