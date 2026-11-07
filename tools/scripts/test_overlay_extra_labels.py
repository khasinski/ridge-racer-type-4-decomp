"""Two label renderers with live source changes, native cursors and real digits."""
from overlay_units import read_c_source
from overlay_names import name as OVN
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest


class ExtraLabelsTest(unittest.TestCase):
    def test_native_labels(self):
        root = Path(__file__).resolve().parents[2]
        bodies = '\n'.join(read_c_source(root / f'src/overlays/PAL/{OVN(o)}/func_{a}.c')
                           for o,a in ((663,'80116744'),(662,'80118238')))
        bodies = re.sub(r'__asm__\("\$2"\)', '', bodies)
        formatter = read_c_source(root / 'tools/scripts/fixtures/views/func_8002B814.c')
        source = r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4 || sizeof(t)==n,"PAL size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4 || offsetof(t,f)==n,"PAL offset")
#include "rr/overlay_extra_labels.h"
typedef struct {
 u32 guard0[4],rng;RrBadgeRow1C rows[4];RrUiUvRect uv;
 s32 counts[9],index,tick;s8 selected;u8 letters[8];u32 guard1[4];
} State;
typedef struct {s32 op,x,y,a,b,c,d,e;size_t cursor;u32 before;u8 data[8];} Event;
typedef struct {State s;unsigned count;Event events[32];} World;
static World actual,reference,initial,*w;
static u8 arena[1024];static u32 ordering[4];
RrBadgeRow1C *BadgeRows;
#define records_record_label w->s.uv
#define records_selected_record w->s.index
#define records_course_unlock_levels w->s.counts
#define garage_D_801273A8 w->s.letters
#define garage_D_801273A0 w->s.selected
#define ScreenFrame w->s.tick
''' + bodies + formatter + r'''
static u32 next(void){u32 x=w->s.rng;x^=x<<13;x^=x>>17;x^=x<<5;return w->s.rng=x;}
static u32 hash(void){const u8 *p=(const u8*)&w->s;u32 h=2166136261U;size_t i;for(i=0;i<sizeof(w->s);i++)h=(h^p[i])*16777619U;return h;}
static void change(void){unsigned i;BadgeRows=w->s.rows+next()%4;
 w->s.index=next()%9;w->s.selected=(s8)next();w->s.tick=(s32)next();
 for(i=0;i<9;i++)w->s.counts[i]=next()%100;
 for(i=0;i<8;i++){u32 n=next()%100;w->s.letters[i]=n<10?0:32+n%90;}
 w->s.uv.u=next();w->s.uv.v=next();w->s.uv.w=next();w->s.uv.h=next();
}
static Event *event(s32 op,void *p){Event *e;assert(w->count<32);e=&w->events[w->count++];memset(e,0,sizeof(*e));e->op=op;e->before=hash();if(p)e->cursor=(u8*)p-arena;return e;}
s32 rcos(s32 angle){Event *e=event(1,0);s32 v=(s32)(next()%3)-1;e->a=angle;e->b=v;change();return v;}
void *AddSpritePrim(u32 *ot,void *p,s32 x,s32 y,const RrUiUvRect *uv,u16 clut,s32 color){Event *e=event(2,p);assert(ot==ordering);e->x=x;e->y=y;e->a=clut;e->b=color;memcpy(e->data,uv,4);change();return (u8*)p+20;}
void *AddTextSprites(u32 *ot,void *p,s32 x,s32 y,const char *text,s32 clut,s32 spacing,s32 color){Event *e=event(3,p);size_t n=strlen(text);assert(n<8&&ot==ordering);e->x=x;e->y=y;e->a=clut;e->b=spacing;e->c=color;memcpy(e->data,text,n+1);change();return (u8*)p+20*n;}
void *AddTilePrim(u32 *ot,void *p,s32 x,s32 y,s32 width,s32 height,s32 r,s32 g,s32 b){Event *e=event(4,p);assert(ot==ordering);e->x=x;e->y=y;e->a=width;e->b=height;e->c=r;e->d=g;e->e=b;change();return (u8*)p+16;}
void *AddDrawModePrim(u32 *ot,void *p,u16 mode){Event *e=event(5,p);assert(ot==ordering);e->a=mode;change();return (u8*)p+12;}
static void *expected(unsigned kind,s32 row){
 int x=BadgeRows[row].x04+8,y=BadgeRows[row].y06+(kind?72:32);void *p=arena;
 if(!kind){char text[8]={0};int n;p=AddSpritePrim(ordering,p,x,y,&w->s.uv,32760,0);x+=w->s.uv.w;n=w->s.counts[w->s.index];text[0]='0'+n/10;text[1]='0'+n%10;p=AddTextSprites(ordering,p,x,y,text,32760,0,0);}
 else {int i;for(i=0;i<8;i++){
  char text[4]={(char)w->s.letters[i],0,0,0};int color=255,clut=32760;
  if(i==w->s.selected){int v=rcos((s32)((u32)w->s.tick*128U-682U));if(v<0){color=8;clut=32761;}}
  p=AddTilePrim(ordering,p,x+i*12,y,8,2,color,color,color);
  p=AddTextSprites(ordering,p,x+i*12+2,y-20,text,clut,0,0);
 }}
 return AddDrawModePrim(ordering,p,56);
}
int main(void){unsigned scenario,kind,i;void *got,*want;
 assert((uintptr_t)arena>UINT32_MAX);
 for(scenario=0;scenario<131072;scenario++)for(kind=0;kind<2;kind++){
  memset(&initial,0,sizeof(initial));w=&initial;w->s.rng=0x16744+scenario*17+kind;change();
  w->s.selected=(s8)(scenario%10-1);
  for(i=0;i<4;i++){w->s.rows[i].x04=(s16)next();w->s.rows[i].y06=(s16)next();}
  actual=reference=initial;
  w=&actual;BadgeRows=w->s.rows;got=kind?0:records_draw_record_labels(ordering,arena,scenario%4);if(kind)garage_func_80118238(ordering,arena,scenario%4);
  w=&reference;BadgeRows=w->s.rows;want=expected(kind,scenario%4);
  assert(kind||got==want);assert(!memcmp(&actual,&reference,sizeof(actual)));
 }
 return 0;
}
'''
        with TemporaryDirectory() as d:
            p = Path(d)
            (p / 'test.c').write_text(source)
            run = subprocess.run(['cc', '-std=gnu11', '-O2', '-fsanitize=undefined',
                                  '-fno-sanitize-recover=all', '-I', str(root / 'include'),
                                  str(p / 'test.c'), '-o', str(p / 'test')], capture_output=True, text=True)
            self.assertEqual(run.returncode, 0, run.stderr)
            subprocess.run([str(p / 'test')], check=True)
