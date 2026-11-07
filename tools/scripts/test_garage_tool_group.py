"""Five tool actions: declarative navigation rules and pre-sound state changes."""
from pathlib import Path
from tempfile import TemporaryDirectory
import re
import subprocess
import unittest

class Overlay662ToolGroupTest(unittest.TestCase):
    def test_all_five_actions_modes_and_byte_transitions(self):
        root=Path(__file__).resolve().parents[2]
        names=['80120004','8012019C','8012031C','8012049C','80120648']
        paths=['src/overlays/PAL/garage/func_8011E734.c','src/overlays/PAL/garage/func_8011E694.c','tools/scripts/fixtures/views/func_8004F218.c','tools/scripts/fixtures/views/func_8004F24C.c']+[f'src/overlays/PAL/garage/func_{n}.c' for n in names]
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
static struct World {u32 guard[4];RrGarageSelectionPrefix garage;RrGarageWorkingStorage work;RrGarageButton buttons[46];RrGarageRectangle *table[46];u32 tail[4];} world,expected,map_checkpoint,sound_checkpoint;
static unsigned scenario,family,slot,initial_byte,expected_map,expected_sound,map_calls,sound_calls,seen[5][256];static s32 mapped_mode;
#define garage_D_801276A8 world.garage
#define garage_D_80127668 world.work
#define garage_D_801273B8 world.table
'''+bodies+r'''
typedef struct {unsigned field,index;s32 value;} Set;
#define END {2,0,0}
static const Set moves[5][4][3]={
 {{{0,0,3},{1,3,3},END},{{1,0,1},{0,1,0},{1,3,5}},{{1,1,1},{1,2,0},END},{{1,1,3},{1,2,3},END}},
 {{{0,0,4},{1,0,0},{1,3,1}},{{0,1,1},{1,3,3},END},{{1,1,4},{1,2,3},END},{{1,1,2},{1,2,1},END}},
 {{{0,0,4},{1,0,0},{1,3,4}},{{0,1,1},{1,3,6},END},{{1,1,1},{1,2,0},END},{{1,1,3},{1,2,2},END}},
 {{{0,1,0},{1,3,2},END},{{0,2,0},{1,0,2},{1,3,4}},{{1,1,4},{1,2,3},END},{{1,1,2},{1,2,1},END}},
 {{{0,1,0},{1,3,5},END},{{0,2,0},{1,0,2},{1,3,7}},{{1,1,1},{1,2,0},END},{{1,1,3},{1,2,2},END}}
};
static const unsigned negative_index[]={0,1,1,1,1},negative_value[]={4,0,0,1,1};
static const unsigned second_index[]={5,4,5,4,5},second_value[]={0,1,0,1,0};
static const s32 defaults[5][5]={{10,0,2,2,4},{11,1,1,0,2},{12,1,2,1,5},{13,1,1,0,3},{14,1,2,1,6}};
static u32 rng=0x20004;
static u32 next(void){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng;}
static s32 word(u32 n){s32 v;memcpy(&v,&n,4);return v;}
static void map_effect(struct World *w){w->garage.optionIndices98.flat[0]=word(scenario^0x87654321U);w->garage.selectorsB8[2]=word(scenario^0x12345678U);w->garage.config1E=255;w->work.words[0]^=0xA5678901U;}
static void sound_effect(struct World *w){unsigned n=(unsigned)((RrGarageButton*)w->table[slot]-world.buttons);assert(n<46);
 w->table[slot]=&world.buttons[(n+1)%46].rectangle;w->garage.active21=(u8)(scenario+41);w->garage.config1D=(u8)(scenario+43);w->garage.config1E=(u8)(scenario+47);w->garage.saved64[0]=word(scenario^0x12345678U);w->work.words[0]^=0x98765432U;
}
void garage_func_8011E5C0(s32 mode){assert(expected_map==1&&map_calls++==0&&mode==mapped_mode);assert(memcmp(&world,&map_checkpoint,sizeof(world))==0);map_effect(&world);}
void TriggerSfx(s32 program,s32 level,s32 pan,s32 pitch,s32 voice){
 assert(expected_sound==1&&sound_calls++==0);assert(program==1&&level==127&&pan==64&&pitch==0x1E00&&voice==-1);assert(memcmp(&world,&sound_checkpoint,sizeof(world))==0);
 seen[family][initial_byte]=1;sound_effect(&world);
}
static void reference(s32 mode){unsigned j,m,n,value;
 if(mode<0){expected.garage.optionIndices98.flat[negative_index[family]]=(s32)negative_value[family];return;}
 if(mode>0){
  if(mode==1)m=0;else if(mode==2)m=1;else if(mode==4)m=2;else if(mode==8)m=3;else return;
  for(j=0;j<3;j++){const Set *s=&moves[family][m][j];if(s->field==0)expected.garage.optionIndices98.flat[s->index]=s->value;else if(s->field==1)expected.garage.selectorsB8[s->index]=s->value;}
  mapped_mode=m>=2?1:0;expected_map=1;memcpy(&map_checkpoint,&expected,sizeof(expected));map_effect(&expected);return;
 }
 if(!(scenario&4)||!(scenario&2))return;
 if(scenario&1){expected.garage.saved64[0]=expected.garage.current60;for(j=0;j<4;j++)expected.garage.saved64[j+1]=expected.garage.selectorsB8[j];}
 else{expected.garage.optionIndices98.flat[negative_index[family]]=(s32)negative_value[family];expected.garage.optionIndices98.flat[second_index[family]]=(s32)second_value[family];memcpy(expected.garage.saved64,defaults[family],sizeof(defaults[family]));}
 if(family==0||family==3){n=(unsigned)((RrGarageButton*)expected.table[slot]-world.buttons);expected.buttons[n].timer0E=7;}
 if(family==0)expected.garage.active21=initial_byte==0?1:0;
 else if(family==3){value=(initial_byte*2)%256;expected.garage.config1D=(u8)(value>=9?1:value);}
 else expected.garage.config1E=(u8)(family==4?3:family);
 expected_sound=1;memcpy(&sound_checkpoint,&expected,sizeof(expected));sound_effect(&expected);
}
int main(void){unsigned path,j,n;uint64_t cases=0;void (*functions[5])(s32)={garage_func_80120004,garage_func_8012019C,garage_func_8012031C,garage_func_8012049C,garage_func_80120648};
 for(scenario=0;scenario<65536;scenario++)for(family=0;family<5;family++)for(path=0;path<8;path++){
  s32 mode;slot=10+family;initial_byte=(scenario>>3)&255;memset(&world,0,sizeof(world));
  for(j=0;j<4;j++){world.guard[j]=next();world.tail[j]=next();world.garage.selectorsB8[j]=word(next());}
  for(j=0;j<8;j++)world.garage.optionIndices98.flat[j]=word(next());for(j=0;j<5;j++)world.garage.saved64[j]=word(next());world.garage.current60=word(next());world.garage.config1E=(u8)next();world.garage.active21=(u8)initial_byte;world.garage.config1D=(u8)initial_byte;
  for(j=0;j<16;j++)world.work.words[j]=next();world.work.words[0]=0xA5510000U|((scenario&1)?4U:0U);world.work.words[1]=0xABC00000U|((scenario&2)?1U:0U);
  for(j=0;j<46;j++){world.buttons[j].key0C=(s16)j;world.buttons[j].timer0E=(s16)(100+j);world.table[j]=&world.buttons[j].rectangle;}
  n=(scenario+slot)%46;world.table[slot]=&world.buttons[n].rectangle;world.buttons[n].rectangle[2]=320;world.buttons[n].rectangle[3]=480;
  world.work.points.cursor14=(RrGaragePoint){(scenario&4)?146:-1,199};
  switch(path){case 0:mode=word(0x80000000U|scenario|(scenario<<16));break;case 1:mode=0;break;case 2:mode=1;break;case 3:mode=2;break;case 4:mode=4;break;case 5:mode=8;break;case 6:mode=3;break;default:mode=scenario%4==0?65536:scenario%4==1?INT32_MAX:word(next());break;}
  expected_map=expected_sound=map_calls=sound_calls=0;memcpy(&expected,&world,sizeof(world));reference(mode);functions[family](mode);
  assert(map_calls==expected_map&&sound_calls==expected_sound);assert(memcmp(&world,&expected,sizeof(world))==0);assert((uintptr_t)world.table[slot]>UINT32_MAX);cases++;
 }
 for(n=0;n<5;n++)for(j=0;j<256;j++)assert(seen[n][j]);assert(cases==2621440ULL);return 0;
}
'''
        with TemporaryDirectory() as tmp:
            p=Path(tmp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
