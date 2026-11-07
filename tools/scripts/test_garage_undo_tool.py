"""Undo action: complete owners, real backup/hit/audio helpers and fresh native pointers."""
from pathlib import Path
from tempfile import TemporaryDirectory
import re
import subprocess
import unittest

class GarageUndoToolTest(unittest.TestCase):
    def test_navigation_and_fresh_pointer_pair_swaps(self):
        root=Path(__file__).resolve().parents[2]
        paths=['src/overlays/PAL/garage/func_8011E734.c','src/overlays/PAL/garage/func_8011E694.c','src/overlays/PAL/garage/func_8011D120.c','tools/scripts/fixtures/views/func_8004F218.c','tools/scripts/fixtures/views/func_8004F24C.c','src/overlays/PAL/garage/func_801207C8.c']
        bodies='\n'.join(re.sub(r'^extern[^\n]*;\n','',(root/p).read_text(),flags=re.M) for p in paths)
        bodies=re.sub(r' __asm__\("\$[0-9]+"\)', '', bodies)
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
#include "rr/garage_tool_action.h"
#include "rr/audio_pan.h"
static struct World {u32 guard[4];RrGarageSelectionPrefix garage;RrGarageWorkingStorage work;RrGarageButton buttons[46];RrGarageRectangle *table[46];u32 pool[4096],palettes[2][8],tail[4];} world,expected,map_checkpoint,sound_checkpoint;
typedef union {u32 words[528];RrGarageTimBytes tim;} TimOwner;
static TimOwner tims[4],timSnapshot[4];
static unsigned selectedSeen[2][2],navigationSeen[8];
static unsigned scenario,expected_map,expected_sound,map_calls,sound_calls;static s32 mapped_mode,displacement;
#define garage_logo_state world.garage
#define garage_logo_work world.work
#define garage_logo_buttons world.table
'''+bodies+r'''
static u32 rng=0x1F290;
static u32 next(void){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng;}
static s32 word(u32 n){s32 v;memcpy(&v,&n,4);return v;}
static unsigned offset(u32 *p){ptrdiff_t n=p-world.pool;assert(n>=0&&n<=4096-512);return (unsigned)n;}
static void map_effect(struct World *w){w->garage.optionIndices98.flat[0]=word(scenario^0x87654321U);w->garage.selectorsB8[2]=word(scenario^0x12345678U);w->work.words[0]^=0xA5678901U;}
static void sound_effect(struct World *w){unsigned active=(scenario>>8)&1;selectedSeen[w->garage.active21][active]++;
 w->table[15]=&world.buttons[17].rectangle;w->garage.active21=(u8)active;
 w->garage.buffers24[active]=tims[(scenario>>6)&1].words;
 w->garage.tim2C[active]=&tims[2+((scenario>>7)&1)].tim;
 w->garage.image38[active]=world.pool+1500;
 w->garage.image40[active]=world.pool+1500+displacement;
 w->garage.palette48[active]=world.palettes[(scenario>>6)&1];
 w->garage.paletteBackup50[active]=world.palettes[(scenario>>7)&1];
 w->garage.saved64[0]=word(scenario^0x12345678U);w->work.words[0]^=0x98765432U;
}
void garage_navigate_to_option(s32 mode){assert(expected_map==1&&map_calls++==0&&mode==mapped_mode);assert(memcmp(&world,&map_checkpoint,sizeof(world))==0);map_effect(&world);}
void TriggerSfx(s32 program,s32 level,s32 pan,s32 pitch,s32 voice){
 assert(expected_sound==1&&sound_calls++==0);assert(program==1&&level==127&&pan==64&&pitch==0x1E00&&voice==-1);
 assert(memcmp(&world,&sound_checkpoint,sizeof(world))==0);assert(world.buttons[15].timer0E==7);sound_effect(&world);
}
static void backup(void){unsigned active=expected.garage.active21,other=!active,j,s,d;
 expected.garage.dirtyD3=1;
 s=offset(expected.garage.image38[active]);d=offset(expected.garage.image38[other]);for(j=0;j<512;j++)expected.pool[d+j]=expected.pool[s+j];
 s=offset(expected.garage.image40[active]);d=offset(expected.garage.image40[other]);for(j=0;j<512;j++)expected.pool[d+j]=expected.pool[s+j];
 for(j=0;j<8;j++)expected.palettes[other][j]=expected.palettes[active][j];
}
static void reference(s32 mode){unsigned j,active;u32 *saved;
 if(mode<0)return;
 if(mode>0){
  switch(mode){
   case 1:expected.garage.selectorsB8[0]=2;expected.garage.selectorsB8[3]=2;mapped_mode=0;break;
   case 2:expected.garage.selectorsB8[0]=1;expected.garage.selectorsB8[3]=1;mapped_mode=0;break;
   case 4:expected.garage.selectorsB8[1]=3;expected.garage.selectorsB8[2]=3;mapped_mode=1;break;
   case 8:
    navigationSeen[scenario%8]++;
    switch(expected.garage.optionIndices98.flat[0]){
     case 1:case 2:expected.garage.optionIndices98.flat[3]=0;
     case 0:case 3:expected.garage.selectorsB8[1]=0;break;
     case 4:expected.garage.selectorsB8[1]=1;break;
    }
    expected.garage.selectorsB8[2]=0;mapped_mode=1;break;
   default:return;
  }
  expected_map=1;memcpy(&map_checkpoint,&expected,sizeof(expected));map_effect(&expected);return;
 }
 if(!(scenario&8)||!(scenario&16))return;
 if(scenario&4){expected.garage.saved64[0]=expected.garage.current60;for(j=0;j<4;j++)expected.garage.saved64[j+1]=expected.garage.selectorsB8[j];}
 else{static const s32 defaults[]={15,0,4,4,0};expected.garage.optionIndices98.flat[0]=4;expected.garage.optionIndices98.flat[7]=0;memcpy(expected.garage.saved64,defaults,sizeof(defaults));}
 backup();expected.buttons[15].timer0E=7;expected_sound=1;memcpy(&sound_checkpoint,&expected,sizeof(expected));sound_effect(&expected);
 active=expected.garage.active21;
 saved=expected.garage.buffers24[active];expected.garage.buffers24[active]=(u32*)expected.garage.tim2C[active];expected.garage.tim2C[active]=(RrGarageTimBytes*)saved;
 saved=expected.garage.image38[active];expected.garage.image38[active]=expected.garage.image40[active];expected.garage.image40[active]=saved;
 saved=expected.garage.palette48[active];expected.garage.palette48[active]=expected.garage.paletteBackup50[active];expected.garage.paletteBackup50[active]=saved;
}
int main(void){unsigned path,j,k,active;uint64_t cases=0;static const s32 shifts[]={-511,-1,0,1,511,512,1024};
 for(scenario=0;scenario<8192;scenario++)for(path=0;path<8;path++){
  s32 mode;memset(&world,0,sizeof(world));active=scenario&1;displacement=shifts[(scenario>>5)%7];
  for(j=0;j<4;j++){world.guard[j]=next();world.tail[j]=next();world.garage.selectorsB8[j]=word(next());}
  for(j=0;j<4096;j++)world.pool[j]=0x12345678U^(scenario*73U+j*17);
  world.garage.image38[0]=world.pool+600;world.garage.image38[1]=world.pool+600+displacement;
  world.garage.image40[0]=world.pool+2400;world.garage.image40[1]=world.pool+2400+displacement;
  for(j=0;j<2;j++){world.garage.buffers24[j]=tims[j].words;world.garage.tim2C[j]=&tims[2+j].tim;world.garage.paletteBackup50[j]=world.palettes[!j];world.garage.palette48[j]=world.palettes[j];for(k=0;k<8;k++)world.palettes[j][k]=next();}
  for(j=0;j<8;j++)world.garage.optionIndices98.flat[j]=word(next());for(j=0;j<5;j++)world.garage.saved64[j]=word(next());world.garage.optionIndices98.flat[0]=(s32)(scenario%8)-2;world.garage.current60=word(next());world.garage.active21=(u8)active;world.garage.layer23=(scenario&2)?255:0;
  for(j=0;j<16;j++)world.work.words[j]=next();world.work.words[0]=0xA5510000U|((scenario&4)?4U:0U);world.work.words[1]=0xABC00000U|((scenario&16)?1U:0U);
  for(j=0;j<46;j++)world.table[j]=&world.buttons[j].rectangle;
  world.buttons[15].rectangle[2]=320;world.buttons[15].rectangle[3]=480;world.buttons[15].timer0E=-123;world.buttons[17].timer0E=99;
  world.work.points.cursor14=(RrGaragePoint){(scenario&8)?146:-1,199};
  switch(path){case 0:mode=word(0x80000000U|scenario|(scenario<<16));break;case 1:mode=0;break;case 2:mode=1;break;case 3:mode=2;break;case 4:mode=4;break;case 5:mode=8;break;case 6:mode=3;break;default:mode=scenario%4==0?65536:scenario%4==1?INT32_MAX:word(next());break;}
  memcpy(timSnapshot,tims,sizeof(tims));expected_map=expected_sound=map_calls=sound_calls=0;memcpy(&expected,&world,sizeof(world));reference(mode);garage_swap_layers_action(mode);
  assert(map_calls==expected_map&&sound_calls==expected_sound);assert(memcmp(&world,&expected,sizeof(world))==0);assert(memcmp(tims,timSnapshot,sizeof(tims))==0);assert((uintptr_t)world.pool>UINT32_MAX);assert((uintptr_t)tims>UINT32_MAX);cases++;
 }
 assert(cases==65536ULL);for(j=0;j<8;j++)assert(navigationSeen[j]);for(j=0;j<2;j++)for(k=0;k<2;k++)assert(selectedSeen[j][k]);return 0;
}
'''
        with TemporaryDirectory() as tmp:
            p=Path(tmp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
