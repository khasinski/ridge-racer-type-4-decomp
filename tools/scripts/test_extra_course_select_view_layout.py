"""Resource arena publication, four glyph callbacks and seven UI rows."""
from overlay_units import read_c_source
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

class Overlay666ViewLayoutTest(unittest.TestCase):
    def test_fresh_glyph_and_selected_value_after_layout_calls(self):
        root=Path(__file__).resolve().parents[2]
        body=read_c_source(root/'src/overlays/PAL/extra_course_select/func_801148D4.c')
        reset=re.sub(r'^extern[^\n]*;\n','',read_c_source(root/'tools/scripts/fixtures/legacy/func_8003A198.c'),flags=re.M)
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
#include "rr/overlay_menu_init.h"
typedef struct {s32 kind,args[10];RrMenuArenas arenas;u8 *base;void *published;RrMenuGlyphRow1C *glyph;s32 frame,counter;s16 current,limit,selected;s8 flag1,flag2;} Event;
static struct World {u32 guard[4];s32 frame,counter;s16 current,limit,selected;s8 flag1,flag2;RrMenuArenas arenas;u8 *base;void *published;RrMenuGlyphRow1C *glyph;RrMenuGlyphRow1C glyphs[3][4];RrUiRow24 rows[7];u32 calls;Event events[13];u32 tail[4];} world,expected;
static u8 resources[3][0x2D100],original_resources[3][0x2D100];
#define ModeFrame world.frame
#define ScreenFrame world.counter
#define MenuCursor world.current
#define MenuItemCount world.limit
#define CourseListIndex world.selected
#define LoadState world.flag1
#define MenuScriptStep world.flag2
#define MenuArenas world.arenas
#define CdDirectoryBuffer world.base
#define LoadCursor world.published
#define GlyphRows world.glyph
''' + reset + body + r'''
static u32 seed=0x148d4,scenario;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static s16 half(u32 raw){u16 bits=raw;s16 value;memcpy(&value,&bits,2);return value;}
static s32 word(u32 raw){s32 value;memcpy(&value,&raw,4);return value;}
static void effect(struct World *v,s32 kind,const s32 args[10]){
 unsigned step=v->calls,id=(scenario+step)%3;RrMenuGlyphRow1C *g=&v->glyphs[id][0];
 v->glyph=world.glyphs[id];v->base=resources[(scenario+step)%3];v->arenas.second=resources[(scenario+step+1)%3]+128+step;v->published=resources[(scenario+step+2)%3]+256+step;
 v->frame=word(scenario^0xA1234567U^step);v->counter=word(scenario^0xB1234567U^step);v->current=half(scenario+step);v->limit=half(scenario*73U+step);v->selected=half(scenario*40503U+step*17U);v->flag1=(s8)(step+9);v->flag2=(s8)(step+11);
 g->glyph07=(u8)(scenario+step);g->x08=half(scenario+step);g->y0A=half(scenario*73U+step);g->w0C=half(scenario*40503U+step);g->h0E=half(scenario*251U+step);
 if(kind==3){RrUiRow24 *r=&v->rows[args[0]];r->half04=half((u32)args[2]);r->half06=half((u32)args[3]);r->half08=half((u32)args[4]);r->byte01=(u8)args[9];}
}
static void observe(struct World *v,s32 kind,const s32 args[10]){
 unsigned i;Event *event;assert(v->calls<13);event=&v->events[v->calls++];event->kind=kind;
 for(i=0;i<10;i++)event->args[i]=args[i];event->arenas=v->arenas;event->base=v->base;event->published=v->published;event->glyph=v->glyph;event->frame=v->frame;event->counter=v->counter;event->current=v->current;event->limit=v->limit;event->selected=v->selected;event->flag1=v->flag1;event->flag2=v->flag2;effect(v,kind,args);
}
void InitMenuArenas(void){s32 args[10]={0};assert(world.calls==0 && world.frame==0 && world.counter==0 && world.current==0 && world.limit==3 && world.flag1==0 && world.flag2==0 && world.arenas.second==world.base+0x2D000);observe(&world,0,args);}
void InitGlyphRow(s32 index,s32 control,s32 glyph,s32 style){s32 args[10]={index,control,glyph,style};assert(world.calls>=1 && world.calls<=4 && index==(s32)world.calls-1);if(world.calls==1)assert(world.published==world.arenas.second);observe(&world,1,args);}
void InitRect16Row(s32 index,s16 a,s16 b,s16 c,s16 d,s16 e,s16 f){s32 args[10]={index,a,b,c,d,e,f};assert(world.calls==5);observe(&world,2,args);}
void InitUiRow(s32 a,s32 b,s32 c,s32 d,s32 e,s32 f,s32 g,s32 h,s32 i,s32 j,...){s32 args[10]={a,b,c,d,e,f,g,h,i,j};assert(world.calls>=6 && world.calls<13 && a==(s32)world.calls-6);observe(&world,3,args);}
static const s32 rows[7][10]={{0,12,64,68,32,0,0,1,0,83},{1,12,104,68,32,0,0,1,0,85},{2,12,368,248,32,0,0,4,12,0},{3,24,88,68,32,0,0,1,0,84},{4,24,128,68,32,0,0,1,0,85},{5,116,88,28,32,0,0,1,0,55},{6,116,128,28,32,0,0,1,0,56}};
static void reference(void){
 unsigned i;RrMenuGlyphRow1C *g;s32 args[10]={0};
 expected.frame=0;expected.counter=0;expected.current=0;expected.limit=3;expected.flag1=0;expected.flag2=0;expected.arenas.second=expected.base+0x2D000;
 observe(&expected,0,args);expected.published=expected.arenas.second;
 for(i=0;i<4;i++){s32 a[10]={(s32)i,-2,0,1};observe(&expected,1,a);}
 g=&expected.glyphs[(scenario+5)%3][0];
 {s32 a[10]={g->glyph07,g->x08,g->y0A,g->w0C,g->h0E,0,0};observe(&expected,2,a);}
 for(i=0;i<7;i++){memcpy(args,rows[i],sizeof(args));if(i==2)args[9]=(s32)expected.selected+8;observe(&expected,3,args);}
}
int main(void){unsigned k,i;
 for(i=0;i<sizeof(resources);i++)((u8*)resources)[i]=(u8)next();memcpy(original_resources,resources,sizeof(resources));
 for(k=0;k<65536;k++){
  for(i=0;i<sizeof(world);i++)((u8*)&world)[i]=(u8)next();
  scenario=k;world.calls=0;world.arenas.first=resources[k%3]+64;world.arenas.second=resources[(k+1)%3]+96;world.base=resources[(k+2)%3];world.published=resources[k%3]+128;world.glyph=world.glyphs[k%3];
  assert((uintptr_t)world.base>UINT32_MAX && (uintptr_t)world.glyph>UINT32_MAX);
  memcpy(&expected,&world,sizeof(world));reference();extra_course_select_init_menu();assert(memcmp(&world,&expected,sizeof(world))==0);
 }
 assert(memcmp(resources,original_resources,sizeof(resources))==0);
 return 0;
}
'''
        with TemporaryDirectory() as temp:
            p=Path(temp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
