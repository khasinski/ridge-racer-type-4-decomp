"""Mode/zoom actions: genuine button timers and post-sound byte wrapping."""
from pathlib import Path
from tempfile import TemporaryDirectory
import re
import struct
import subprocess
import unittest

class Overlay662ToolCyclesTest(unittest.TestCase):
    def test_mode_and_zoom_actions_all_postcall_byte_values(self):
        root=Path(__file__).resolve().parents[2]
        blob=(root/'disc/PAL/R4.BIN').read_bytes();sector=struct.unpack_from('<I',blob,4+662*4)[0]
        rows=[struct.unpack_from('<8h',blob,sector*2048+0x80126F88-0x80114878+16*i) for i in range(46)]
        self.assertEqual(sorted(r[6] for r in rows),list(range(46)))
        data='static const RrGarageButton original[46]={\n'+',\n'.join('{{'+','.join(map(str,r[:4]))+'},'+','.join(map(str,r[4:]))+'}' for r in rows)+'\n};\n'
        paths=['src/overlays/PAL/garage/func_8011E734.c','src/overlays/PAL/garage/func_8011E694.c','tools/scripts/fixtures/views/func_8004F218.c','tools/scripts/fixtures/views/func_8004F24C.c','src/overlays/PAL/garage/func_8011FB2C.c','src/overlays/PAL/garage/func_8011FCE0.c']
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
#include "rr/overlay662_tool_action.h"
#include "rr/audio_pan.h"
_Static_assert(sizeof(RrGarageButton)==16,"complete button owner");
static struct World {u32 guard[4];RrGarageSelectionPrefix garage;RrGarageWorkingStorage work;RrGarageButton buttons[46];RrGarageRectangle *table[46];u32 tail[4];} world,expected,map_checkpoint,sound_checkpoint;
static unsigned scenario,family,slot,postbyte,expected_map,expected_sound,map_calls,sound_calls,seen[2][256];static s32 mapped_mode;
#define garage_logo_state world.garage
#define garage_logo_work world.work
#define garage_logo_buttons world.table
'''+data+bodies+r'''
static u32 rng=0x1FB2C;
static u32 next(void){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng;}
static s32 word(u32 n){s32 v;memcpy(&v,&n,4);return v;}
static s16 half(u32 n){u16 bits=(u16)n;s16 v;memcpy(&v,&bits,2);return v;}
static unsigned button_slot(RrGarageRectangle *p){return (unsigned)((RrGarageButton*)p-world.buttons);}
static void map_effect(struct World *w){w->garage.optionIndices98.flat[0]=word(scenario^0x87654321U);w->garage.selectorsB8[2]=word(scenario^0x12345678U);w->garage.config1E=255;w->work.words[0]^=0xA5678901U;}
static void sound_effect(struct World *w){unsigned n=button_slot(w->table[slot]);assert(n<46);
 w->table[slot]=&world.buttons[(n+1)%46].rectangle;
 w->garage.config1F=(u8)postbyte;w->garage.config1C=(u8)postbyte;
 w->garage.reset08[0]=half(scenario*73U+17);w->garage.reset08[1]=half(scenario*251U+19);
 w->garage.saved64[0]=word(scenario^0x12345678U);w->work.words[0]^=0x98765432U;
}
void garage_navigate_to_option(s32 mode){assert(expected_map==1&&map_calls++==0&&mode==mapped_mode);assert(memcmp(&world,&map_checkpoint,sizeof(world))==0);map_effect(&world);}
void TriggerSfx(s32 program,s32 level,s32 pan,s32 pitch,s32 voice){
 assert(expected_sound==1&&sound_calls++==0);assert(program==1&&level==127&&pan==64&&pitch==0x1E00&&voice==-1);
 assert(memcmp(&world,&sound_checkpoint,sizeof(world))==0);assert(((RrGarageButton*)world.table[slot])->timer0E==7);
 seen[family][postbyte]=1;sound_effect(&world);
}
static void reference(s32 mode){unsigned j,n,value;
 if(mode<0){expected.garage.optionIndices98.flat[0]=3;return;}
 if(mode>0){
  switch(mode){
   case 1:expected.garage.optionIndices98.flat[0]=2;if(!family)expected.garage.selectorsB8[1]=0;expected.garage.selectorsB8[3]=2;mapped_mode=0;break;
   case 2:expected.garage.optionIndices98.flat[0]=4;if(!family)expected.garage.selectorsB8[1]=1;expected.garage.selectorsB8[3]=family?4:1;mapped_mode=0;break;
   case 4:expected.garage.selectorsB8[1]=expected.garage.selectorsB8[2]=family?0:4;mapped_mode=1;break;
   case 8:expected.garage.selectorsB8[1]=expected.garage.selectorsB8[2]=family?3:2;mapped_mode=1;break;
   default:return;
  }
  expected_map=1;memcpy(&map_checkpoint,&expected,sizeof(expected));map_effect(&expected);return;
 }
 if(!(scenario&4)||!(scenario&2))return;
 if(scenario&1){expected.garage.saved64[0]=expected.garage.current60;for(j=0;j<4;j++)expected.garage.saved64[j+1]=expected.garage.selectorsB8[j];}
 else{expected.garage.optionIndices98.flat[0]=3;expected.garage.optionIndices98.flat[family?5:3]=0;expected.garage.saved64[0]=(s32)slot;expected.garage.saved64[1]=0;expected.garage.saved64[2]=family?2:0;expected.garage.saved64[3]=family?2:0;expected.garage.saved64[4]=3;}
 n=button_slot(expected.table[slot]);expected.buttons[n].timer0E=7;
 expected_sound=1;memcpy(&sound_checkpoint,&expected,sizeof(expected));sound_effect(&expected);
 if(!family){value=(postbyte+1)%256;expected.garage.config1F=(u8)(value>=3?0:value);}
 else{value=(postbyte*2)%256;expected.garage.config1C=(u8)value;if(value>=17){expected.garage.config1C=2;expected.garage.reset08[0]=expected.garage.reset08[1]=0;}}
}
int main(void){unsigned path,j,n;uint64_t cases=0;
 for(scenario=0;scenario<65536;scenario++)for(family=0;family<2;family++)for(path=0;path<8;path++){
  s32 mode;slot=7+family;postbyte=(scenario>>3)&255;
  memset(&world,0,sizeof(world));memcpy(world.buttons,original,sizeof(original));
  for(j=0;j<4;j++){world.guard[j]=next();world.tail[j]=next();world.garage.selectorsB8[j]=word(next());}
  for(j=0;j<8;j++)world.garage.optionIndices98.flat[j]=word(next());for(j=0;j<5;j++)world.garage.saved64[j]=word(next());world.garage.current60=word(next());world.garage.config1C=(u8)next();world.garage.config1F=(u8)next();world.garage.reset08[0]=half(next());world.garage.reset08[1]=half(next());
  for(j=0;j<16;j++)world.work.words[j]=next();world.work.words[0]=0xA5510000U|((scenario&1)?4U:0U);world.work.words[1]=0xABC00000U|((scenario&2)?1U:0U);
  for(j=0;j<46;j++){world.table[(unsigned)world.buttons[j].key0C]=&world.buttons[j].rectangle;world.buttons[j].timer0E=half(scenario+j);}
  n=scenario%46;world.table[slot]=&world.buttons[n].rectangle;world.buttons[n].rectangle[0]=world.buttons[n].rectangle[1]=0;world.buttons[n].rectangle[2]=320;world.buttons[n].rectangle[3]=480;
  world.work.points.cursor14=(RrGaragePoint){(scenario&4)?146:-1,199};
  switch(path){case 0:mode=word(0x80000000U|scenario|(scenario<<16));break;case 1:mode=0;break;case 2:mode=1;break;case 3:mode=2;break;case 4:mode=4;break;case 5:mode=8;break;case 6:mode=3;break;default:mode=scenario%4==0?65536:scenario%4==1?INT32_MAX:word(next());break;}
  expected_map=expected_sound=map_calls=sound_calls=0;
  memcpy(&expected,&world,sizeof(world));reference(mode);if(family)garage_cycle_zoom(mode);else garage_cycle_alt_button_action(mode);
  assert(map_calls==expected_map&&sound_calls==expected_sound);assert(memcmp(&world,&expected,sizeof(world))==0);assert((uintptr_t)world.table[slot]>UINT32_MAX);cases++;
 }
 for(j=0;j<256;j++)assert(seen[0][j]&&seen[1][j]);assert(cases==1048576ULL);return 0;
}
'''
        with TemporaryDirectory() as tmp:
            p=Path(tmp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
