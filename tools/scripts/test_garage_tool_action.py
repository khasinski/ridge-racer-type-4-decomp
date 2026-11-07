"""Tool action: actual hit/snapshot/sound/distance callers and genuine point views."""
from pathlib import Path
from tempfile import TemporaryDirectory
import re
import subprocess
import unittest

class Overlay662ToolActionTest(unittest.TestCase):
    def test_modes_snapshot_fresh_cursor_and_final_distance_store(self):
        root=Path(__file__).resolve().parents[2]
        paths=['src/overlays/PAL/garage/func_8011E734.c','src/overlays/PAL/garage/func_8011E694.c','tools/scripts/fixtures/views/func_8004F218.c','tools/scripts/fixtures/views/func_8004F24C.c','src/overlays/PAL/garage/func_8011DBD8.c','src/overlays/PAL/garage/func_8011F4C0.c']
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
_Static_assert(sizeof(RrGarageWorkingStorage)==64,"complete work owner");
static struct World {u32 guard[4];RrGarageSelectionPrefix garage;RrGarageWorkingStorage work;RrGarageRectangle rects[46],*table[46];u32 tail[4];} world,expected,map_checkpoint,sound_checkpoint,root_checkpoint;
static unsigned scenario,expected_map,expected_sound,expected_root,map_calls,sound_calls,root_calls;
static s32 mapped_mode,root_argument,root_result;
#define garage_D_801276A8 world.garage
#define garage_D_80127668 world.work
#define garage_D_801273B8 world.table
'''+bodies+r'''
static u32 rng=0x1F4C0;
static u32 next(void){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng;}
static s32 word(u32 n){s32 v;memcpy(&v,&n,4);return v;}
static s16 half(u32 n){u16 bits=(u16)n;s16 v;memcpy(&v,&bits,2);return v;}
static void map_effect(struct World *w){w->garage.optionIndices98.flat[0]=word(scenario^0x87654321U);w->garage.selectorsB8[2]=word(scenario^0x12345678U);w->garage.config1E=255;w->work.words[0]^=0xA5678901U;}
static void sound_effect(struct World *w){
 w->garage.config1E=(u8)(scenario+7);w->garage.saved64[0]=word(scenario^0x12345678U);
 w->work.words[0]=scenario^0xABCDEF01U;w->work.points.cursor14.x=half(scenario*73U+17);w->work.points.cursor14.y=half(scenario*251U+19);
}
static void root_effect(struct World *w){w->work.points.word28=0x13572468U;w->work.points.first20.x=half(scenario+11);w->work.points.second24.y=half(scenario+13);w->work.words[0]^=0x98765432U;}
void garage_func_8011E5C0(s32 mode){assert(expected_map==1&&map_calls++==0&&mode==mapped_mode);assert(memcmp(&world,&map_checkpoint,sizeof(world))==0);map_effect(&world);}
void TriggerSfx(s32 program,s32 level,s32 pan,s32 pitch,s32 voice){
 assert(expected_sound==1&&sound_calls++==0);assert(program==3&&level==127&&pan==64&&pitch==0x1E00&&voice==-1);
 assert(memcmp(&world,&sound_checkpoint,sizeof(world))==0);sound_effect(&world);
}
s32 CordicSqrt(s32 squared){assert(expected_root==1&&root_calls++==0&&squared==root_argument);assert(memcmp(&world,&root_checkpoint,sizeof(world))==0);root_effect(&world);return root_result;}
static void reference(s32 mode){unsigned j;int64_t dx,dy,sum;
 if(mode<0){expected.garage.optionIndices98.flat[0]=2;expected.garage.optionIndices98.flat[3]=0;return;}
 if(mode>0){
  switch(mode){
   case 1:expected.garage.optionIndices98.flat[0]=1;expected.garage.selectorsB8[3]=1;mapped_mode=0;break;
   case 2:expected.garage.optionIndices98.flat[0]=3;expected.garage.selectorsB8[3]=3;mapped_mode=0;break;
   case 4:expected.garage.selectorsB8[1]=4;expected.garage.selectorsB8[2]=4;mapped_mode=1;break;
   case 8:expected.garage.optionIndices98.flat[3]=1;expected.garage.selectorsB8[2]=1;mapped_mode=1;break;
   default:return;
  }
  expected_map=1;memcpy(&map_checkpoint,&expected,sizeof(expected));map_effect(&expected);return;
 }
 /* Fixture chooses either a strict interior point or x=-1 for a0,0,320,480 rectangle. */
 if(!(scenario&4)||!(scenario&2))return;
 if(scenario&1){expected.garage.saved64[0]=expected.garage.current60;for(j=0;j<4;j++)expected.garage.saved64[j+1]=expected.garage.selectorsB8[j];}
 else{static const s32 defaults[]={4,0,0,0,2};expected.garage.optionIndices98.flat[0]=2;expected.garage.optionIndices98.flat[3]=0;memcpy(expected.garage.saved64,defaults,sizeof(defaults));}
 expected.garage.config1E=4;expected_sound=1;memcpy(&sound_checkpoint,&expected,sizeof(expected));sound_effect(&expected);
 expected.work.words[0]|=2;expected.work.points.first20=expected.work.points.cursor14;expected.work.points.second24=(RrGaragePoint){146,199};expected.work.points.word28=0;
 dx=(int64_t)146-expected.work.points.first20.x;dy=(int64_t)199-expected.work.points.first20.y;sum=dx*dx*4+dy*dy;
 root_argument=word((u32)((uint64_t)sum&0xFFFFFFFFU));root_result=(scenario&15)==0?INT32_MAX:(s32)((scenario*40503U)&0x7FFFFFFFU);
 expected_root=1;memcpy(&root_checkpoint,&expected,sizeof(expected));root_effect(&expected);
 expected.work.points.distance2C=(s32)(4096/((int64_t)root_result/4096+1));
}
int main(void){unsigned path,j,k;uint64_t cases=0;
 for(scenario=0;scenario<65536;scenario++)for(path=0;path<8;path++){
  s32 mode;
  memset(&world,0,sizeof(world));
  for(j=0;j<4;j++){world.guard[j]=next();world.tail[j]=next();world.garage.selectorsB8[j]=word(next());}
  for(j=0;j<8;j++)world.garage.optionIndices98.flat[j]=word(next());for(j=0;j<5;j++)world.garage.saved64[j]=word(next());world.garage.current60=word(next());world.garage.config1E=(u8)next();
  for(j=0;j<16;j++)world.work.words[j]=next();world.work.words[0]=0xA5510000U|((scenario&1)?4U:0U);world.work.words[1]=0xABC00000U|((scenario&2)?1U:0U);
  for(j=0;j<46;j++){world.table[j]=&world.rects[j];for(k=0;k<4;k++)world.rects[j][k]=(u16)next();}
  world.rects[4][0]=world.rects[4][1]=0;world.rects[4][2]=320;world.rects[4][3]=480;
  world.work.points.cursor14=(RrGaragePoint){(scenario&4)?146:-1,199};
  switch(path){case 0:mode=word(0x80000000U|scenario|(scenario<<16));break;case 1:mode=0;break;case 2:mode=1;break;case 3:mode=2;break;case 4:mode=4;break;case 5:mode=8;break;case 6:mode=3;break;default:mode=scenario%4==0?65536:scenario%4==1?INT32_MAX:word(next());break;}
  expected_map=expected_sound=expected_root=map_calls=sound_calls=root_calls=0;
  memcpy(&expected,&world,sizeof(world));reference(mode);garage_func_8011F4C0(mode);
  assert(map_calls==expected_map&&sound_calls==expected_sound&&root_calls==expected_root);
  assert(memcmp(&world,&expected,sizeof(world))==0);assert((uintptr_t)&world.work.points.first20>UINT32_MAX);cases++;
 }
 assert(cases==524288ULL);return 0;
}
'''
        with TemporaryDirectory() as tmp:
            p=Path(tmp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
