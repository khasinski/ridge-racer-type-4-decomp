"""Garage view setup: reset checkpoint, seven typed boundaries and fresh owners."""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest

class GarageViewSetupTest(unittest.TestCase):
    def test_ordered_setup_with_replaced_glyph_and_badge_roots(self):
        root=Path(__file__).resolve().parents[2]
        body=(root/'src/overlays/PAL/662/func_80116020.c').read_text()
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
#include "rr/overlay662_setup.h"
typedef struct {s32 args[10];RrMenuGlyphRow1C *glyph;RrBadgeRow1C *badge;s32 frame,counter,word;s16 half,setting,selected;u8 choice;} Event;
static struct World {u32 guard[4];s32 frame,counter,word;s16 half,setting,selected;RrMenuChoice4 choice;RrMenuGlyphRow1C *glyph;RrBadgeRow1C *badge;RrMenuGlyphRow1C glyphs[3][4];RrBadgeRow1C badges[3];u32 calls;Event events[7];u32 tail[4];} world,expected;
#define ModeFrame world.frame
#define ScreenFrame world.counter
#define ovl662_D_801272C0 world.word
#define ovl662_D_801272D4 world.half
#define D_800AD520 world.setting
#define CourseListIndex world.selected
#define SelectionRows world.choice
#define GlyphRows world.glyph
#define BadgeRows world.badge
''' + body + r'''
static u32 seed=0x16020,scenario;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static s16 half(u32 raw){u16 bits=raw;s16 value;memcpy(&value,&bits,2);return value;}
static s32 word(u32 raw){s32 value;memcpy(&value,&raw,4);return value;}
static void effect(struct World *v,unsigned step){
 unsigned id=(scenario+step+1)%3;RrMenuGlyphRow1C *g=&v->glyphs[id][0];
 v->glyph=world.glyphs[id];v->badge=&world.badges[id];
 v->frame=word(scenario*0x9e3779b9U+step);v->counter=word(scenario^0xa1234567U^step);v->word=word(scenario^0xb1234567U^step);
 v->half=half(scenario+step);v->setting=half(scenario*40503U+step*17U);v->selected=half(scenario*73U+step);v->choice.bytes[0]=(u8)(scenario+step+23);
 g->glyph07=(u8)(scenario+step);g->x08=half(scenario+step);g->y0A=half(scenario*40503U+17U+step);g->w0C=half(scenario*73U+step);g->h0E=half(scenario*251U+step);
 v->badges[id].half02=half(scenario+step+9);
}
static void event(struct World *v,const s32 args[10]){
 unsigned step=v->calls,i;Event *e;assert(step<7);
 if(step==0)assert(v->frame==0 && v->counter==0 && v->word==0 && v->half==0 && v->setting==2 && v->selected==0 && v->choice.bytes[0]==2);
 e=&v->events[v->calls++];for(i=0;i<10;i++)e->args[i]=args[i];
 e->glyph=v->glyph;e->badge=v->badge;e->frame=v->frame;e->counter=v->counter;e->word=v->word;
 e->half=v->half;e->setting=v->setting;e->selected=v->selected;e->choice=v->choice.bytes[0];effect(v,step);
}
/* ABI-confirmed resident setup boundaries. Mutations expose any unintended
 * reset after a call, captured old root, or stale signed halfword argument. */
void InitGlyphRow(s32 index,s32 control,s32 glyph,s32 style){s32 a[10]={index,control,glyph,style};assert(world.calls<4 && index==(s32)world.calls);event(&world,a);}
void InitRect16Row(s32 index,s16 a,s16 b,s16 c,s16 d,s16 e,s16 f){s32 args[10]={index,a,b,c,d,e,f};assert(world.calls==4);event(&world,args);}
void InitUiRow(s32 a,s32 b,s32 c,s32 d,s32 e,s32 f,s32 g,s32 h,s32 i,s32 j,...){s32 args[10]={a,b,c,d,e,f,g,h,i,j};assert(world.calls==5);event(&world,args);}
void InitBadgeRow(s32 a,s32 b,s16 c,s32 d,u16 e,u16 f,u16 g){s32 args[10]={a,b,c,d,e,f,g};assert(world.calls==6);event(&world,args);}
static void reference(void){
 unsigned i,id;RrMenuGlyphRow1C *g;
 expected.frame=0;expected.counter=0;expected.word=0;expected.half=0;expected.setting=2;expected.choice.bytes[0]=2;expected.selected=0;
 for(i=0;i<4;i++){s32 args[10]={(s32)i,-2,0,1};event(&expected,args);}
 id=(scenario+4)%3;g=&expected.glyphs[id][0];
 {s32 args[10]={g->glyph07,g->x08,g->y0A,g->w0C,g->h0E,0,0};event(&expected,args);}
 {s32 args[10]={0,12,72,240,32,0,0,4,2,expected.setting};event(&expected,args);}
 {s32 args[10]={0,36,336,248,34,0,0x301};event(&expected,args);}
 expected.badges[(scenario+7)%3].half02=1;
}
int main(void){unsigned k,i;
 for(k=0;k<65536;k++){
  for(i=0;i<sizeof(world);i++)((u8*)&world)[i]=(u8)next();
  scenario=k;world.calls=0;world.glyph=world.glyphs[k%3];world.badge=&world.badges[(k+1)%3];
  assert((uintptr_t)world.glyph>UINT32_MAX && (uintptr_t)world.badge>UINT32_MAX);
  memcpy(&expected,&world,sizeof(world));reference();ovl662_func_80116020();
  assert(memcmp(&world,&expected,sizeof(world))==0);
 }
 return 0;
}
'''
        with TemporaryDirectory() as temp:
            p=Path(temp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
