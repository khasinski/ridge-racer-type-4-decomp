"""Glyph phase duplication: modulo words, overlapping rows and fresh roots."""
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess
import unittest

class Overlay662GlyphDuplicateTest(unittest.TestCase):
    def test_phase_wrap_aliasing_and_typed_layout_calls(self):
        root=Path(__file__).resolve().parents[2]
        body=(root/'src/overlays/PAL/662/func_80115EC4.c').read_text()
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
#include "rr/overlay662_setup.h"
typedef struct {s32 args[10],phases[12],local;s16 selected;RrMenuChoice4 choice;RrMenuGlyphRow1C *root;RrBadgeRow1C *badge;} Event;
static struct World {
 u32 guard[4];RrMenuGlyphRow1C glyphs[3][12],*root;RrBadgeRow1C badges[3][2],*badge;
 s32 local;s16 selected;RrMenuChoice4 choice;unsigned calls;Event events[10];u32 tail[4];
} world,expected;
static unsigned scenario,base,switching;
#define GlyphRows world.root
#define BadgeRows world.badge
#define ovl662_D_801272C0 world.local
#define CourseListIndex world.selected
#define CourseListSavedIndex world.choice.bytes[1]
'''+body+r'''
static u32 rng=0x15EC4;
static u32 next(void){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng;}
static s32 word(u32 n){s32 v;memcpy(&v,&n,4);return v;}
static s16 half(u32 n){u16 bits=(u16)n;s16 v;memcpy(&v,&bits,2);return v;}
static unsigned bank(RrMenuGlyphRow1C *root){unsigned b=0;while(root!=world.glyphs[b]){b++;assert(b<3);}return b;}
static void event(struct World *w,const s32 a[10]){unsigned n=w->calls,j,b=bank(w->root),newbank=switching?(scenario+n+1)%3:b;Event *e;
 assert(n<10);e=&w->events[w->calls++];for(j=0;j<10;j++)e->args[j]=a[j];for(j=0;j<12;j++)e->phases[j]=w->glyphs[b][j].phase18;
 e->local=w->local;e->selected=w->selected;e->choice=w->choice;e->root=w->root;e->badge=w->badge;
 w->local=word(scenario*0x9E3779B9U+n);w->selected=half(scenario*73U+n);
 for(j=0;j<4;j++)w->choice.bytes[j]=(u8)(scenario+n+j*17);
 w->root=world.glyphs[newbank];w->badge=world.badges[(scenario+n)%3];
 for(j=0;j<12;j++){
  RrMenuGlyphRow1C *g=&w->glyphs[newbank][j];g->glyph07=(u8)(scenario+n+j);
  g->x08=half(scenario+j+n);g->y0A=half(scenario*73U+j+n);g->w0C=half(scenario*19U+j+n);g->h0E=half(scenario*251U+j+n);
  if(switching)g->phase18=word(scenario*0x87654321U+n+j);
 }
 for(j=0;j<sizeof(w->badges);j++)((u8*)w->badges)[j]=(u8)(scenario+n+j*13);
}
void InitGlyphRow(s32 index,s32 control,s32 glyph,s32 style){s32 a[10]={index,control,glyph,style};
 assert(world.calls<8&&world.calls%2==0&&index==(s32)(base+world.calls/2));
 if(world.calls==0)assert(world.local==0&&world.selected==0&&world.choice.bytes[1]==0);event(&world,a);
}
void InitRect16Row(s32 index,s16 a,s16 b,s16 c,s16 d,s16 e,s16 f){s32 args[10]={index,a,b,c,d,e,f};assert(world.calls<8&&world.calls%2==1);event(&world,args);}
void InitBadgeRow(s32 a,s32 b,s16 c,s32 d,u16 e,u16 f,u16 g){s32 args[10]={a,b,c,d,e,f,g};assert(world.calls==8);event(&world,args);}
void InitUiRow(s32 a,s32 b,s32 c,s32 d,s32 e,s32 f,s32 g,s32 h,s32 i,s32 j,...){s32 args[10]={a,b,c,d,e,f,g,h,i,j};assert(world.calls==9);event(&world,args);}
static void reference(void){unsigned i,b;RrMenuGlyphRow1C *g;
 expected.local=0;expected.selected=0;expected.choice.bytes[1]=0;
 for(i=0;i<4;i++){
  int64_t wide;u32 bits;
  b=bank(expected.root);wide=(int64_t)expected.glyphs[b][i].phase18+196608;
  bits=(u32)((uint64_t)wide&0xFFFFFFFFU);expected.glyphs[b][base+i].phase18=word(bits);
  {s32 a[10]={(s32)(base+i),-2,0,1};event(&expected,a);}
  b=bank(expected.root);g=&expected.glyphs[b][base+i];
  {s32 a[10]={g->glyph07,g->x08,g->y0A,g->w0C,g->h0E,0,0};event(&expected,a);}
 }
 {s32 a[10]={0,108,336,112,34,0,0x401};event(&expected,a);}
 {s32 a[10]={0,40,72,192,32,0,0,4,3,(s32)expected.selected+4};event(&expected,a);}
 expected.badges[(scenario+9)%3][0].half02=1;
}
int main(void){unsigned b,j,k;
 static const u32 edges[]={0,0x7FFFFFFFU,0x80000000U,0xFFFFFFFFU,0x7FFD0000U,0x7FFCFFFFU,0xFFFD0000U,0x12345678U};
 for(scenario=0;scenario<65536;scenario++){
  memset(&world,0,sizeof(world));base=scenario%9;switching=(scenario>>4)&1;
  for(j=0;j<4;j++){world.guard[j]=next();world.tail[j]=next();world.choice.bytes[j]=(u8)next();}
  for(b=0;b<3;b++)for(j=0;j<12;j++){
   RrMenuGlyphRow1C *g=&world.glyphs[b][j];for(k=0;k<sizeof(*g);k++)((u8*)g)[k]=(u8)next();
   g->phase18=word(scenario%3==0?edges[(scenario+b+j)%8]:next());
  }
  world.local=word(next());world.selected=half(next());world.root=world.glyphs[scenario%3];world.badge=world.badges[(scenario+1)%3];
  memcpy(&expected,&world,sizeof(world));reference();ovl662_func_80115EC4((s32)base);
  assert(memcmp(&world,&expected,sizeof(world))==0);assert((uintptr_t)world.root>UINT32_MAX&&(uintptr_t)world.badge>UINT32_MAX);
 }
 return 0;
}
'''
        with TemporaryDirectory() as tmp:
            p=Path(tmp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
