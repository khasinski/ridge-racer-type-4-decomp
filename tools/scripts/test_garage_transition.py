"""Transition wrappers, defined counter wrap and the real glyph reset closure."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

PREFIX=r'''
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4 || sizeof(t)==n,"target size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4 || offsetof(t,f)==n,"target offset")
#include "rr/overlay662_transition.h"
'''

class Overlay662TransitionTest(unittest.TestCase):
    def run_native(self,root,source):
        with TemporaryDirectory() as temp:
            p=Path(temp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

    def test_fresh_renderer_state_and_modulo_counter(self):
        root=Path(__file__).resolve().parents[2]
        bodies='\n'.join((root/f'src/overlays/PAL/garage/func_{a}.c').read_text() for a in ['8011AA84','80119DB4'])
        source=PREFIX+r'''
typedef struct {u32 guard[4];RrGarageSelectionPrefix garage;u16 mode;s16 scene,phase;u32 tail[4];} State;
typedef struct {s32 kind,arg;State state;} Event;
static struct World {State s;u32 calls;Event events[3];u32 guard[4];} world,expected;
static u32 resources[3][16];
#define garage_D_801276A8 world.s.garage
#define ModeGroup world.s.mode
#define ModeId world.s.scene
#define ModeSubState world.s.phase
''' + bodies + r'''
static u32 seed=0x1AA84,scenario;static s32 after;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static s32 word(u32 bits){s32 v;memcpy(&v,&bits,4);return v;}
static s16 half(u32 raw){u16 bits=(u16)raw;s16 v;memcpy(&v,&bits,2);return v;}
static void observe(struct World *v,s32 kind,s32 arg){
 Event *e;unsigned i,step=v->calls;assert(step<3);e=&v->events[v->calls++];memset(e,0,sizeof(*e));e->kind=kind;e->arg=arg;memcpy(&e->state,&v->s,sizeof(v->s));
 v->s.garage.word04=kind==2?after:word(scenario*0x9E3779B9U+step);v->s.garage.fade00=word(scenario^0xA1234567U^step);v->s.garage.flagsD1=(u8)(scenario+step);v->s.garage.phaseCC=scenario*73U+step;
 v->s.mode=(u16)(scenario*40503U+step);v->s.scene=half(scenario*251U+step);v->s.phase=half(scenario*73U+step);
 for(i=0;i<2;i++){v->s.garage.image38[i]=resources[(scenario+step+i)%3];v->s.garage.image40[i]=resources[(scenario+step+i+1)%3];v->s.garage.palette48[i]=resources[(scenario+step+i+2)%3];}
}
void garage_func_8011B484(void){assert(world.calls==0);observe(&world,0,0);}
void garage_func_8011B400(void){assert(world.calls==1);observe(&world,1,0);}
void garage_func_8011CF8C(void){assert(world.calls==2);observe(&world,2,0);}
void garage_func_80119B44(void){assert(world.calls==0);observe(&world,3,0);}
void UpdateScreenFade(s32 flags){assert(world.calls==1 && flags==0x10002 && world.s.mode==1 && world.s.scene==70 && world.s.phase==0);observe(&world,4,flags);}
static void reference_transition(void){
 int64_t counter;
 observe(&expected,0,0);observe(&expected,1,0);observe(&expected,2,0);
 counter=(int64_t)expected.s.garage.word04+1;if(counter>INT32_MAX)counter=INT32_MIN;
 expected.s.garage.word04=(s32)counter;
 if(counter>=28){expected.s.mode=3;expected.s.scene=5;expected.s.phase=0;}
}
static void initialize(void){unsigned i,j;for(i=0;i<sizeof(world);i++)((u8*)&world)[i]=(u8)next();world.calls=0;
 for(i=0;i<2;i++){world.s.garage.image38[i]=resources[i];world.s.garage.image40[i]=resources[i+1];world.s.garage.palette48[i]=resources[i];}
 for(j=0;j<8;j++)world.s.garage.options78.flat[j]=0;
 memcpy(&expected,&world,sizeof(world));}
int main(void){unsigned k;static const s32 edges[]={26,27,28,INT32_MAX,INT32_MIN,-1,0,INT32_MAX-1};
 for(k=0;k<65536;k++){
  scenario=k;after=k<8?edges[k]:word(k*0x9E3779B9U);
  initialize();reference_transition();garage_func_8011AA84();assert(memcmp(&world,&expected,sizeof(world))==0);
  initialize();observe(&expected,3,0);expected.s.mode=1;expected.s.scene=70;expected.s.phase=0;observe(&expected,4,0x10002);garage_func_80119DB4();assert(memcmp(&world,&expected,sizeof(world))==0);
 }
 return 0;
}
'''
        self.run_native(root,source)

    def test_first_arena_and_full_signed_glyph_components(self):
        root=Path(__file__).resolve().parents[2]
        reset=re.sub(r'^extern[^\n]*;\n','',(root/'tools/scripts/fixtures/legacy/func_8003A198.c').read_text(),flags=re.M)
        encoder=(root/'tools/scripts/fixtures/legacy/func_80037040.c').read_text()
        body=(root/'src/overlays/PAL/garage/func_80115474.c').read_text()
        source=PREFIX+r'''
static struct World {u32 guard[4];RrMenuArenas arenas;void *published;RrMenuChoice4 choice;s16 glyph;u8 *mode;u8 backing[3][2];s32 counter;s8 flag1,flag2;u32 tail[4];} world,expected;
static u8 resources[3][64];
#define MenuArenas world.arenas
#define LoadCursor world.published
#define SelectionRows world.choice
#define D_800ABE2C world.glyph
#define ImageRows world.mode
#define ScreenFrame world.counter
#define LoadState world.flag1
#define MenuScriptStep world.flag2
''' + reset + encoder + body + r'''
static u32 seed=0x15474;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static s16 half(u32 raw){u16 bits=(u16)raw;s16 v;memcpy(&v,&bits,2);return v;}
int main(void){unsigned k,i,alias,id;
 for(k=0;k<65536;k++){
  int64_t glyph;
  for(i=0;i<sizeof(world);i++)((u8*)&world)[i]=(u8)next();world.glyph=half(k);glyph=world.glyph;alias=k%4;id=(k/4)%3;
  world.arenas.first=resources[k%3]+17;world.arenas.second=resources[(k+1)%3]+23;world.published=resources[(k+2)%3]+29;
  world.mode=alias?&world.choice.bytes[alias-1]:world.backing[id];
  assert((uintptr_t)world.arenas.first>UINT32_MAX && (uintptr_t)world.mode>UINT32_MAX);
  memcpy(&expected,&world,sizeof(world));expected.counter=0;expected.flag1=0;expected.flag2=0;expected.published=expected.arenas.first;
  expected.choice.bytes[0]=(u8)(glyph/160);expected.choice.bytes[1]=(u8)((glyph/40)-(glyph/160)*4);expected.choice.bytes[2]=(u8)(glyph%40);
  if(alias)expected.choice.bytes[alias]=6;else expected.backing[id][1]=6;
  garage_func_80115474();assert(memcmp(&world,&expected,sizeof(world))==0);
 }
 return 0;
}
'''
        self.run_native(root,source)

if __name__=='__main__':unittest.main()
