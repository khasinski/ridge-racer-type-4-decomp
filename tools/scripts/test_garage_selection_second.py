"""Garage byte selection in a genuine40-row UI bank, with real sound closure."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

class Overlay662SelectionSecondTest(unittest.TestCase):
    def test_flags_priorities_and_full_signed_callback_selection(self):
        root=Path(__file__).resolve().parents[2]
        body=(root/'src/overlays/PAL/garage/func_80116808.c').read_text()
        bridge=(root/'tools/scripts/fixtures/views/func_8004F218.c').read_text().replace('#include "rr/audio_pan.h"','')
        sound=re.sub(r'^extern[^\n]*;\n','',(root/'tools/scripts/fixtures/views/func_8004F24C.c').read_text(),flags=re.M)
        source=r'''
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4 || sizeof(t)==n,"target size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4 || offsetof(t,f)==n,"target offset")
#include "rr/menu_ui_bank.h"
static struct World {u32 guard[4];s16 selection;RrMenuChoice4 choice;u16 buttons;s32 direction;RrUiRow24 *ui;RrMenuGlyphRow1C *glyphs;RrMenuUiBank backing[3];RrMenuGlyphRow1C records[3][4];u32 calls;s32 sound_args[5];u32 tail[4];} world,expected;
#define CourseListIndex world.selection
#define CourseListSavedIndex world.choice.bytes[1]
#define SelectionRows world.choice
#define Pad0RepeatButtons world.buttons
#define MenuStepDirection world.direction
#define UiRows world.ui
#define GlyphRows world.glyphs
void TriggerSfx(s32 program,s32 level,s32 pan,s32 pitch,s32 voice);
''' + bridge + sound + body + r'''
static u32 seed=0x15148;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static s16 half(u32 raw){u16 bits=raw;s16 value;memcpy(&value,&bits,2);return value;}
static s32 signed_word(u32 raw){s32 value;memcpy(&value,&raw,4);return value;}
static unsigned old_root,new_root,new_glyphs,initial;static s16 after;
static void effect(struct World *v,s32 a,s32 b,s32 c,s32 d,s32 e){
 assert(v->calls==0 && a==0 && b==127 && c==64 && d==0x1e00 && e==-1);
 assert(v->backing[old_root].bytes[12+initial]==0);
 v->calls++;v->sound_args[0]=a;v->sound_args[1]=b;v->sound_args[2]=c;v->sound_args[3]=d;v->sound_args[4]=e;
 v->choice.bytes[0]^=0xA5;v->choice.bytes[1]^=0x5A;v->selection=after;v->buttons^=0xA55A;v->direction=signed_word((u32)v->direction^0xFEDCBA98U);
 v->ui=world.backing[new_root].rows;v->glyphs=world.records[new_glyphs];
}
void TriggerSfx(s32 a,s32 b,s32 c,s32 d,s32 e){effect(&world,a,b,c,d,e);}
static void reference(void){
 int moved=0,index=initial;
 expected.selection=initial;
 expected.backing[old_root].bytes[12+initial]=0;
 if((expected.buttons&0x2000) && index<3){index++;moved=2;}
 else if((expected.buttons&0x8000) && index>0){index--;moved=1;}
 expected.direction=moved;
 if(moved){
  expected.selection=index;effect(&expected,0,127,64,0x1e00,-1);
  if(expected.selection<0)expected.selection=0;else if(expected.selection>3)expected.selection=3;
  expected.choice.bytes[1]=(u8)expected.selection;
  expected.records[new_glyphs][expected.selection].control02=-6;

 }
 expected.backing[moved?new_root:old_root].rows[0].byte16=(u8)(expected.selection+4);
 expected.backing[moved?new_root:old_root].bytes[12+expected.selection]=2;
}
int main(void){unsigned k,i;u16 flags;
 for(k=0;k<394240;k++){
  for(i=0;i<sizeof(world);i++)((u8*)&world)[i]=(u8)next();
  if(k<262144){initial=k&3;flags=k>>2;after=half(k*40503U+17U);}
  else if(k<393216){unsigned n=k-262144;initial=(n&1)?3:0;flags=(n&1)?0x8000:0x2000;after=half(n>>1);}
  else{unsigned n=k-393216;static const u16 keys[4]={0,0x2000,0x8000,0xA000};initial=n>>2;flags=keys[n&3];after=half(n*40503U+17U);}
  old_root=k%3;new_root=(k>>2)%3;new_glyphs=(k>>4)%3;
  world.selection=half(next());world.choice.bytes[1]=(u8)initial;world.buttons=flags;world.calls=0;world.ui=world.backing[old_root].rows;world.glyphs=world.records[(k>>6)%3];
  assert((uintptr_t)world.ui>UINT32_MAX && (uintptr_t)world.glyphs>UINT32_MAX);
  memcpy(&expected,&world,sizeof(world));reference();
  garage_func_80116808();assert(memcmp(&world,&expected,sizeof(world))==0);
 }
 return 0;
}
'''
        with TemporaryDirectory() as temp:
            p=Path(temp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
