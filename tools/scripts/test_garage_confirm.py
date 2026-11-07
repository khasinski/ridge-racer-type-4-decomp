"""Confirmation action: navigation, fade handshake and fresh UI roots after audio."""
from pathlib import Path
from tempfile import TemporaryDirectory
import re
import struct
import subprocess
import unittest

class Overlay662ConfirmTest(unittest.TestCase):
    def test_navigation_handshake_and_post_audio_roots(self):
        root=Path(__file__).resolve().parents[2]
        blob=(root/'disc/PAL/R4.BIN').read_bytes();sector=struct.unpack_from('<I',blob,4+662*4)[0]
        rows=[struct.unpack_from('<8h',blob,sector*2048+0x80126F88-0x80114878+16*i) for i in range(46)]
        self.assertEqual(sorted(r[6] for r in rows),list(range(46)))
        data='static const RrGarageButton original[46]={\n'+',\n'.join('{{'+','.join(map(str,r[:4]))+'},'+','.join(map(str,r[4:]))+'}' for r in rows)+'\n};\n'
        paths=['src/overlays/PAL/garage/func_8011E734.c','src/overlays/PAL/garage/func_8011E694.c','tools/scripts/fixtures/views/func_8004F218.c','tools/scripts/fixtures/views/func_8004F24C.c','src/overlays/PAL/garage/func_8011F888.c']
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
#include "rr/overlay662_confirm.h"
#include "rr/audio_pan.h"
_Static_assert(sizeof(RrGarageButton)==16,"complete button owner");
static struct World {u32 guard[4];RrGarageSelectionPrefix garage;RrGarageWorkingStorage work;RrGarageButton buttons[46];RrGarageRectangle *table[46];RrMenuTextState text[3],*textroot;RrBadgeRow1C badges[3][2],*badgeroot;RrMenuItemState5C items[2];s16 pending,scene,latch;u16 transition;u32 tail[4];} world,expected,map_checkpoint,sound_checkpoint;
static unsigned scenario,family,slot,postbyte,expected_map,expected_sound,map_calls,sound_calls;static s32 mapped_mode;
#define garage_logo_state world.garage
#define garage_logo_work world.work
#define garage_logo_buttons world.table
#define DialogueState world.textroot
#define BadgeRows world.badgeroot
#define PlayerControllers world.items
#define MenuResultCode world.pending
#define ModeId world.scene
#define ModeSubState world.latch
#define ModeGroup world.transition
'''+data+bodies+r'''
static u32 rng=0x1FB2C;
static u32 next(void){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng;}
static s32 word(u32 n){s32 v;memcpy(&v,&n,4);return v;}
static s16 half(u32 n){u16 bits=(u16)n;s16 v;memcpy(&v,&bits,2);return v;}
static unsigned button_slot(RrGarageRectangle *p){return (unsigned)((RrGarageButton*)p-world.buttons);}
static void map_effect(struct World *w){w->garage.optionIndices98.flat[0]=word(scenario^0x87654321U);w->garage.selectorsB8[2]=word(scenario^0x12345678U);w->garage.config1E=255;w->work.words[0]^=0xA5678901U;}
static void sound_effect(struct World *w){unsigned n=button_slot(w->table[slot]);assert(n<46);
 w->table[slot]=&world.buttons[(n+1)%46].rectangle;
 w->garage.config1F=(u8)postbyte;w->garage.config1C=(u8)postbyte;w->garage.config22=(u8)postbyte;
 w->garage.reset08[0]=half(scenario*73U+17);w->garage.reset08[1]=half(scenario*251U+19);
 w->garage.saved64[0]=word(scenario^0x12345678U);w->work.words[0]^=0x98765432U;
 w->textroot=world.text+(scenario+1)%3;w->badgeroot=world.badges[(scenario+2)%3];w->garage.fade00=word(scenario^0xA1234567U);w->garage.flagsD1=(u8)scenario;
}
void garage_navigate_to_option(s32 mode){assert(expected_map==1&&map_calls++==0&&mode==mapped_mode);assert(memcmp(&world,&map_checkpoint,sizeof(world))==0);map_effect(&world);}
void TriggerSfx(s32 program,s32 level,s32 pan,s32 pitch,s32 voice){
 assert(expected_sound==1&&sound_calls++==0);assert(program==1&&level==127&&pan==64&&pitch==0x1E00&&voice==-1);
 assert(memcmp(&world,&sound_checkpoint,sizeof(world))==0);
 sound_effect(&world);
}
static unsigned text_index(RrMenuTextState*p){unsigned i;for(i=0;i<3;i++)if(p==world.text+i)return i;assert(0);return 0;}
static unsigned badge_index(RrBadgeRow1C*p){unsigned i;for(i=0;i<3;i++)if(p==world.badges[i])return i;assert(0);return 0;}
static void sound(void){expected_sound=1;memcpy(&sound_checkpoint,&expected,sizeof(expected));sound_effect(&expected);}
static void reference(s32 mode){unsigned j,t,b;
 if(mode<0){expected.garage.optionIndices98.flat[0]=2;return;}
 if(mode>0){switch(mode){
 case 1:expected.garage.optionIndices98.flat[0]=1;expected.garage.selectorsB8[3]=1;mapped_mode=0;break;
 case 2:expected.garage.optionIndices98.flat[0]=3;expected.garage.selectorsB8[3]=3;mapped_mode=0;break;
 case 4:expected.garage.optionIndices98.flat[3]=1;expected.garage.selectorsB8[1]=0;expected.garage.selectorsB8[2]=1;mapped_mode=1;break;
 case 8:expected.garage.selectorsB8[1]=3;expected.garage.selectorsB8[2]=3;mapped_mode=1;break;
 default:return;}
 expected_map=1;map_checkpoint=expected;map_effect(&expected);return;}
 b=badge_index(expected.badgeroot);t=text_index(expected.textroot);
 if(scenario&8){if(expected.badges[b][0].state10!=3)return;
  if(expected.garage.fade00<0){expected.garage.flagsD1|=4;
   if(expected.text[t].st==4&&expected.items[0].type01!=0x12&&expected.items[0].a!=0){expected.garage.fade00=64;sound();}
  }else if(expected.garage.fade00==0){expected.pending=6;expected.transition=1;expected.scene=71;expected.latch=0;}return;
 }
 if(!(scenario&4)||!(scenario&2))return;
 if(scenario&1){expected.garage.saved64[0]=expected.garage.current60;for(j=0;j<4;j++)expected.garage.saved64[j+1]=expected.garage.selectorsB8[j];}
 else{expected.garage.optionIndices98.flat[0]=2;expected.garage.optionIndices98.flat[5]=0;expected.garage.saved64[0]=6;expected.garage.saved64[1]=0;expected.garage.saved64[2]=2;expected.garage.saved64[3]=2;expected.garage.saved64[4]=2;}
 expected.work.words[4]=6;expected.work.words[0]|=8;sound();expected.work.words[0]|=1;
 t=text_index(expected.textroot);b=badge_index(expected.badgeroot);expected.text[t].b4=4;expected.text[t].b5=-1;expected.text[t].st=0;expected.garage.flagsD1=1;expected.badges[b][0].half02=1;expected.badges[b][0].state10=0;
}
int main(void){unsigned path,j,n;uint64_t cases=0;
 for(scenario=0;scenario<65536;scenario++)for(family=0;family<1;family++)for(path=0;path<8;path++){
  s32 mode;slot=6;postbyte=(scenario>>3)&255;
  memset(&world,0,sizeof(world));memcpy(world.buttons,original,sizeof(original));
  for(j=0;j<4;j++){world.guard[j]=next();world.tail[j]=next();world.garage.selectorsB8[j]=word(next());}
  for(j=0;j<8;j++)world.garage.optionIndices98.flat[j]=word(next());for(j=0;j<5;j++)world.garage.saved64[j]=word(next());world.garage.current60=word(next());world.garage.config1C=(u8)next();world.garage.config1F=(u8)next();world.garage.config22=(u8)(255-postbyte);world.garage.reset08[0]=half(next());world.garage.reset08[1]=half(next());
  for(j=0;j<16;j++)world.work.words[j]=next();world.work.words[0]=0xA5510000U|((scenario&1)?4U:0U)|((scenario&8)?8U:0U);world.work.words[1]=0xABC00000U|((scenario&2)?1U:0U);
  for(j=0;j<46;j++){world.table[(unsigned)world.buttons[j].key0C]=&world.buttons[j].rectangle;world.buttons[j].timer0E=half(scenario+j);}
  n=scenario%46;world.table[slot]=&world.buttons[n].rectangle;world.buttons[n].rectangle[0]=world.buttons[n].rectangle[1]=0;world.buttons[n].rectangle[2]=320;world.buttons[n].rectangle[3]=480;
  world.work.points.cursor14=(RrGaragePoint){(scenario&4)?146:-1,199};
  switch(path){case 0:mode=word(0x80000000U|scenario|(scenario<<16));break;case 1:mode=0;break;case 2:mode=1;break;case 3:mode=2;break;case 4:mode=4;break;case 5:mode=8;break;case 6:mode=3;break;default:mode=scenario%4==0?65536:scenario%4==1?INT32_MAX:word(next());break;}
  world.textroot=world.text+scenario%3;world.badgeroot=world.badges[scenario%3];
  for(j=0;j<sizeof(world.text);j++)((u8*)world.text)[j]=(u8)(scenario+j*7);
  for(j=0;j<sizeof(world.badges);j++)((u8*)world.badges)[j]=(u8)(scenario+j*13);
  for(j=0;j<3;j++)world.text[j].p=0;
  world.text[scenario%3].st=(scenario&32)?4:-128;world.badges[scenario%3][0].state10=(scenario&16)?3:-1;
  world.items[0].type01=(scenario&256)?0x12:255;world.items[0].a=(scenario&512)?0:word(0x87654321U);
  switch((scenario>>6)&3){case 0:world.garage.fade00=INT32_MIN;break;case 1:world.garage.fade00=-1;break;case 2:world.garage.fade00=0;break;default:world.garage.fade00=INT32_MAX;break;}
  world.garage.flagsD1=(u8)(scenario>>8);world.pending=half(next());world.transition=(u16)next();world.scene=half(next());world.latch=half(next());
  expected_map=expected_sound=map_calls=sound_calls=0;
  memcpy(&expected,&world,sizeof(world));reference(mode);garage_func_8011F888(mode);
  assert(map_calls==expected_map&&sound_calls==expected_sound);assert(memcmp(&world,&expected,sizeof(world))==0);assert((uintptr_t)world.table[slot]>UINT32_MAX);cases++;
 }
 assert(cases==524288ULL);return 0;
}
'''
        with TemporaryDirectory() as tmp:
            p=Path(tmp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
