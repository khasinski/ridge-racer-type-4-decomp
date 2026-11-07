"""Four-position UI selection, real sound closure and replaced native owners."""
from overlay_units import read_c_source
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

class Overlay666SelectionTest(unittest.TestCase):
    def test_flags_priorities_and_full_signed_callback_selection(self):
        root=Path(__file__).resolve().parents[2]
        body=read_c_source(root/'src/overlays/PAL/666/func_80115148.c')
        bridge=read_c_source(root/'tools/scripts/fixtures/views/func_8004F218.c').replace('#include "rr/audio_pan.h"','')
        sound=re.sub(r'^extern[^\n]*;\n','',read_c_source(root/'tools/scripts/fixtures/views/func_8004F24C.c'),flags=re.M)
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
#include "rr/overlay_menu.h"
static struct World {u32 guard[4];s16 selection;u16 buttons;s32 direction;RrUiRow24 *ui;RrMenuGlyphRow1C *glyphs;RrUiRow24 backing[3][3];RrMenuGlyphRow1C records[3][4];u32 calls;s32 sound_args[5];u32 tail[4];} world,expected;
#define CourseListIndex world.selection
#define D_800F3A42 world.buttons
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
 assert(v->backing[old_root][2].cells[initial]==0);
 v->calls++;v->sound_args[0]=a;v->sound_args[1]=b;v->sound_args[2]=c;v->sound_args[3]=d;v->sound_args[4]=e;
 v->selection=after;v->buttons^=0xA55A;v->direction=signed_word((u32)v->direction^0xFEDCBA98U);
 v->ui=world.backing[new_root];v->glyphs=world.records[new_glyphs];
}
void TriggerSfx(s32 a,s32 b,s32 c,s32 d,s32 e){effect(&world,a,b,c,d,e);}
static void reference(void){
 int moved=0,index=initial;
 expected.backing[old_root][2].cells[initial]=0;
 if((expected.buttons&0x2000) && index<3){index++;moved=2;}
 else if((expected.buttons&0x8000) && index>0){index--;moved=1;}
 expected.direction=moved;
 if(moved){
  expected.selection=index;effect(&expected,0,127,64,0x1e00,-1);
  if(expected.selection<0)expected.selection=0;else if(expected.selection>3)expected.selection=3;
  expected.records[new_glyphs][expected.selection].control02=-6;
  expected.records[new_glyphs][expected.selection].flags00=0;
  expected.backing[new_root][2].byte16=(u8)expected.selection+8;
 }
 expected.backing[moved?new_root:old_root][2].cells[expected.selection]=2;
}
int main(void){unsigned k,i;u16 flags;
 for(k=0;k<393216;k++){
  for(i=0;i<sizeof(world);i++)((u8*)&world)[i]=(u8)next();
  if(k<262144){initial=k&3;flags=k>>2;after=half(k*40503U+17U);}
  else{unsigned n=k-262144;initial=(n&1)?3:0;flags=(n&1)?0x8000:0x2000;after=half(n>>1);}
  old_root=k%3;new_root=(k>>2)%3;new_glyphs=(k>>4)%3;
  world.selection=initial;world.buttons=flags;world.calls=0;world.ui=world.backing[old_root];world.glyphs=world.records[(k>>6)%3];
  assert((uintptr_t)world.ui>UINT32_MAX && (uintptr_t)world.glyphs>UINT32_MAX);
  memcpy(&expected,&world,sizeof(world));reference();
  ovl666_update_course_selection();assert(memcmp(&world,&expected,sizeof(world))==0);
 }
 return 0;
}
'''
        with TemporaryDirectory() as temp:
            p=Path(temp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
