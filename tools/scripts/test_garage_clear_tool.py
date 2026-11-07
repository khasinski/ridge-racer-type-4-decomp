"""Clear tool executes actual empty scan, backup, hit-test and sound wrappers."""
from pathlib import Path
from tempfile import TemporaryDirectory
import re
import subprocess
import unittest

class GarageClearToolTest(unittest.TestCase):
    def test_empty_rejection_backup_and_fresh_clear_destination(self):
        root=Path(__file__).resolve().parents[2]
        paths=['src/overlays/PAL/garage/func_8011E734.c','src/overlays/PAL/garage/func_8011E694.c','src/overlays/PAL/garage/func_8011D0C0.c','src/overlays/PAL/garage/func_8011D120.c','tools/scripts/fixtures/views/func_8004F218.c','tools/scripts/fixtures/views/func_8004F24C.c','src/overlays/PAL/garage/func_8011F678.c']
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
#include "rr/garage_tool_action.h"
#include "rr/audio_pan.h"
static struct World {u32 guard[4];RrGarageSelectionPrefix garage;RrGarageWorkingStorage work;RrGarageButton buttons[46];RrGarageRectangle *table[46];u32 images[4][512],palettes[2][8],tail[4];} world,expected,map_checkpoint,sound_checkpoint;
static unsigned scenario,expected_map,expected_sound,map_calls,sound_calls,audio_mode;static s32 mapped_mode;
#define garage_logo_state world.garage
#define garage_logo_work world.work
#define garage_logo_buttons world.table
'''+bodies+r'''
static u32 rng=0x1F678;
static u32 next(void){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng;}
static s32 word(u32 n){s32 v;memcpy(&v,&n,4);return v;}
static unsigned image_slot(u32 *p){unsigned n=0;while(p!=world.images[n]){n++;assert(n<4);}return n;}
static void map_effect(struct World *w){w->garage.optionIndices98.flat[0]=word(scenario^0x87654321U);w->garage.selectorsB8[2]=word(scenario^0x12345678U);w->work.words[0]^=0xA5678901U;}
static void sound_effect(struct World *w){unsigned active=(scenario>>6)&1,layer=(scenario>>7)&1,target=(scenario>>8)&3;
 w->table[5]=&world.buttons[6].rectangle;w->garage.active21=(u8)active;w->garage.layer23=layer?255:0;
 if(layer)w->garage.image40[active]=world.images[target];else w->garage.image38[active]=world.images[target];
 w->garage.saved64[0]=word(scenario^0x12345678U);w->work.words[0]^=0x98765432U;
}
void garage_navigate_to_option(s32 mode){assert(expected_map==1&&map_calls++==0&&mode==mapped_mode);assert(memcmp(&world,&map_checkpoint,sizeof(world))==0);map_effect(&world);}
void TriggerSfx(s32 program,s32 level,s32 pan,s32 pitch,s32 voice){
 assert(expected_sound==1&&sound_calls++==0);assert((unsigned)program==audio_mode&&level==127&&pan==64&&pitch==0x1E00&&voice==-1);
 assert(memcmp(&world,&sound_checkpoint,sizeof(world))==0);assert(world.buttons[5].timer0E==7);sound_effect(&world);
}
static void backup(void){unsigned active=expected.garage.active21,other=!active,j;
 expected.garage.dirtyD3=1;
 for(j=0;j<512;j++)expected.images[other][j]=expected.images[active][j];
 for(j=0;j<512;j++)expected.images[2+other][j]=expected.images[2+active][j];
 for(j=0;j<8;j++)expected.palettes[other][j]=expected.palettes[active][j];
}
static void reference(s32 mode){unsigned j,active,target;
 if(mode<0){expected.garage.optionIndices98.flat[0]=2;expected.garage.optionIndices98.flat[3]=1;return;}
 if(mode>0){
  switch(mode){
   case 1:expected.garage.optionIndices98.flat[0]=1;expected.garage.selectorsB8[3]=1;mapped_mode=0;break;
   case 2:expected.garage.optionIndices98.flat[0]=3;expected.garage.selectorsB8[3]=3;mapped_mode=0;break;
   case 4:expected.garage.optionIndices98.flat[3]=0;expected.garage.selectorsB8[2]=0;mapped_mode=1;break;
   case 8:expected.garage.selectorsB8[1]=2;expected.garage.selectorsB8[2]=2;mapped_mode=1;break;
   default:return;
  }
  expected_map=1;memcpy(&map_checkpoint,&expected,sizeof(expected));map_effect(&expected);return;
 }
 if(!(scenario&16)||!(scenario&32))return;
 audio_mode=(scenario&8)?4:1;
 if(!(scenario&8)){
  if(scenario&4){expected.garage.saved64[0]=expected.garage.current60;for(j=0;j<4;j++)expected.garage.saved64[j+1]=expected.garage.selectorsB8[j];}
  else{static const s32 defaults[]={5,0,0,1,2};expected.garage.optionIndices98.flat[0]=2;expected.garage.optionIndices98.flat[3]=1;memcpy(expected.garage.saved64,defaults,sizeof(defaults));}
  backup();
 }
 expected.buttons[5].timer0E=7;expected_sound=1;memcpy(&sound_checkpoint,&expected,sizeof(expected));sound_effect(&expected);
 if(!(scenario&8)){active=expected.garage.active21;target=image_slot(expected.garage.layer23?expected.garage.image40[active]:expected.garage.image38[active]);memset(expected.images[target],0,2048);}
}
int main(void){unsigned path,j,k,active,layer,selected;uint64_t cases=0;
 for(scenario=0;scenario<65536;scenario++)for(path=0;path<8;path++){
  s32 mode;memset(&world,0,sizeof(world));active=scenario&1;layer=(scenario>>1)&1;selected=layer*2+active;
  for(j=0;j<4;j++){world.guard[j]=next();world.tail[j]=next();world.garage.selectorsB8[j]=word(next());for(k=0;k<512;k++)world.images[j][k]=0x12345678U^(scenario*73U+k*17+j);}
  memset(world.images[selected],0,2048);if(!(scenario&8))world.images[selected][(scenario>>6)&511]=0x80000001U;
  for(j=0;j<2;j++){world.garage.image38[j]=world.images[j];world.garage.image40[j]=world.images[2+j];world.garage.palette48[j]=world.palettes[j];for(k=0;k<8;k++)world.palettes[j][k]=next();}
  for(j=0;j<8;j++)world.garage.optionIndices98.flat[j]=word(next());for(j=0;j<5;j++)world.garage.saved64[j]=word(next());world.garage.current60=word(next());world.garage.active21=(u8)active;world.garage.layer23=layer?255:0;
  for(j=0;j<16;j++)world.work.words[j]=next();world.work.words[0]=0xA5510000U|((scenario&4)?4U:0U);world.work.words[1]=0xABC00000U|((scenario&32)?1U:0U);
  for(j=0;j<46;j++)world.table[j]=&world.buttons[j].rectangle;
  world.buttons[5].rectangle[2]=320;world.buttons[5].rectangle[3]=480;world.buttons[5].timer0E=-123;world.buttons[6].timer0E=99;
  world.work.points.cursor14=(RrGaragePoint){(scenario&16)?146:-1,199};
  switch(path){case 0:mode=word(0x80000000U|scenario|(scenario<<16));break;case 1:mode=0;break;case 2:mode=1;break;case 3:mode=2;break;case 4:mode=4;break;case 5:mode=8;break;case 6:mode=3;break;default:mode=scenario%4==0?65536:scenario%4==1?INT32_MAX:word(next());break;}
  expected_map=expected_sound=map_calls=sound_calls=0;memcpy(&expected,&world,sizeof(world));reference(mode);garage_clear_layer_action(mode);
  assert(map_calls==expected_map&&sound_calls==expected_sound);assert(memcmp(&world,&expected,sizeof(world))==0);assert((uintptr_t)world.images>UINT32_MAX);cases++;
 }
 assert(cases==524288ULL);return 0;
}
'''
        with TemporaryDirectory() as tmp:
            p=Path(tmp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
