"""History labels: full native cursors, live globals and actual decimal helper."""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest


class HistoryLabelsTest(unittest.TestCase):
    def test_native_labels(self):
        root = Path(__file__).resolve().parents[2]
        bodies = '\n'.join((root / f'src/overlays/PAL/garage/func_{a}.c').read_text()
                           for a in ('80117654', '80117AA4', '80117C98', '80117DFC'))
        formatter = (root / 'tools/scripts/fixtures/views/func_8002B814.c').read_text()
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
#include "rr/overlay662_history_labels.h"
typedef struct {
 u32 guard0[4],rng;
 RrBadgeRow1C rows[4];RrSaveProgressRecord7C save;RrMenuChoice4 choice;
 RrUiUvRect uv[128];s16 counts[4][4],number,total;
 s32 tick;u32 guard1[4];
} State;
typedef struct {s32 op,x,y,a,b,c;size_t cursor;u8 data[8];} Event;
typedef struct {State s;Event events[12];unsigned count;} World;
static World actual,reference,initial,*w;
static u8 arena[512];static u32 ordering[4];static char ratio[8];
RrBadgeRow1C *BadgeRows;
#define SaveProgress w->s.save
#define SelectionRows w->s.choice
#define UiFontGlyphs w->s.uv
#define ScreenFrame w->s.tick
#define garage_team_maker_car_counts w->s.counts
#define garage_D_801272BC w->s.number
#define garage_clut_list_index w->s.total
s8 garage_car_number_label_glyphs[3]={64,25,123},garage_maker_cars_label_glyphs[3]={23,24,-1};
s8 garage_D_80126228[3]={26,25,123},garage_car_color_label_glyphs[4]={42,25,123,-1};
''' + bodies + formatter + r'''
static u32 next(void){u32 x=w->s.rng;x^=x<<13;x^=x>>17;x^=x<<5;return w->s.rng=x;}
static void change(void){
 unsigned i;
 BadgeRows=w->s.rows+(next()%4);
 w->s.choice.bytes[0]=next()%4;w->s.choice.bytes[1]=next()%4;w->s.choice.bytes[2]=next()%99;
 for(i=0;i<128;i++)w->s.uv[i].w=next();
 w->s.number=next()%100;w->s.total=next()%100;
 for(i=0;i<16;i++)w->s.counts[i/4][i%4]=next()%100;
}
static Event *event(s32 op,void *cursor){
 Event *e;assert(w->count<12);e=&w->events[w->count++];memset(e,0,sizeof(*e));
 e->op=op;if(cursor)e->cursor=(u8*)cursor-arena;
 return e;
}
s32 rsin(s32 angle){Event *e=event(1,0);s32 v=(s32)(next()%3)-1;e->a=angle;e->b=v;change();return v;}
void *AddSpritePrim(u32 *ot,void *p,s32 x,s32 y,const RrUiUvRect *uv,u16 clut,s32 color){
 Event *e=event(2,p);assert(ot==ordering);e->x=x;e->y=y;e->a=clut;e->b=color;memcpy(e->data,uv,4);change();return (u8*)p+20;
}
void *AddTextSprites(u32 *ot,void *p,s32 x,s32 y,const char *text,s32 clut,s32 spacing,s32 color){
 Event *e=event(3,p);size_t n=strlen(text);assert(n<8&&ot==ordering);e->x=x;e->y=y;e->a=clut;e->b=spacing;e->c=color;memcpy(e->data,text,n+1);change();return (u8*)p+20*n;
}
void *AddDrawModePrim(u32 *ot,void *p,u16 mode){Event *e=event(4,p);assert(ot==ordering);e->a=mode;change();return (u8*)p+12;}
char *FormatCountOfTotal(s32 first,s32 second,s32 digits){
 Event *e=event(5,0);assert(digits==1);e->a=first;e->b=second;
 ratio[0]='0'+first/10;ratio[1]='0'+first%10;ratio[2]='/';ratio[3]='0'+second/10;ratio[4]='0'+second%10;ratio[5]=0;change();return ratio;
}
static void expected(unsigned kind,u32 *ot,void *p,s32 row){
 int x=BadgeRows[row].x04+8,y=BadgeRows[row].y06+32,flag=0,i;
 const s8 *keys=kind==0?garage_car_number_label_glyphs:kind==1?garage_maker_cars_label_glyphs:kind==2?garage_D_80126228:garage_car_color_label_glyphs;
 if(kind<2){
  for(i=0;i<4;i++){
   RrSelectionRow h=w->s.save.history6C[i];
   if(w->s.choice.bytes[0]==h.selection&&w->s.choice.bytes[1]==h.variant&&(kind||w->s.choice.bytes[2]==h.parameter)){
    flag=rsin((s32)(((u32)w->s.tick*128-682U)&4094U))<0;break;
   }
  }
 }
 for(i=0;i<(kind==3?4:3);i++){
  if(keys[i]<0){
   int n=kind==1?w->s.counts[w->s.choice.bytes[0]][w->s.choice.bytes[1]]:w->s.total;
   char *text=FormatCountOfTotal(n,kind==1?20:24,1);
   p=AddTextSprites(ot,p,x,y,text,32760+flag,0,0);break;
  }
  p=AddSpritePrim(ot,p,x,y,&w->s.uv[(unsigned)keys[i]],32760,0);
  x+=w->s.uv[(unsigned)keys[i]].w+2;
  if(i==2&&(kind==0||kind==2)){
   char text[8]={0};int n=kind==0?w->s.choice.bytes[2]+1:w->s.number;
   text[0]='0'+n/10;text[1]='0'+n%10;
   p=AddTextSprites(ot,p,x,y,text,32760+flag,0,0);
  }
 }
 AddDrawModePrim(ot,p,56);
}
int main(void){unsigned scenario,kind,i;void (*functions[4])(u32*,void*,s32)={garage_draw_car_number_label,garage_draw_maker_count_label,garage_func_80117C98,garage_draw_clut_index_label};
 assert((uintptr_t)arena>UINT32_MAX);_Static_assert(sizeof(RrSaveProgressRecord7C)==124,"full save");
 for(scenario=0;scenario<65536;scenario++)for(kind=0;kind<4;kind++){
  memset(&initial,0,sizeof(initial));w=&initial;w->s.rng=0x17654+scenario*17+kind;change();
  for(i=0;i<4;i++){
   w->s.rows[i].x04=(s16)next();w->s.rows[i].y06=(s16)next();
   w->s.save.history6C[i].selection=next()%4;w->s.save.history6C[i].variant=next()%4;w->s.save.history6C[i].parameter=next()%99;
  }
  if(scenario%5<4){RrSelectionRow *h=&w->s.save.history6C[scenario%5];h->selection=w->s.choice.bytes[0];h->variant=w->s.choice.bytes[1];h->parameter=w->s.choice.bytes[2];}
  w->s.tick=(s32)next();actual=reference=initial;
  w=&actual;BadgeRows=w->s.rows;functions[kind](ordering,arena,scenario%4);
  w=&reference;BadgeRows=w->s.rows;expected(kind,ordering,arena,scenario%4);
  assert(!memcmp(&actual,&reference,sizeof(actual)));
 }
 return 0;
}
'''
        with TemporaryDirectory() as d:
            p = Path(d)
            (p / 'test.c').write_text(source)
            subprocess.run(['cc', '-std=gnu11', '-O2', '-fsanitize=undefined',
                            '-fno-sanitize-recover=all', '-I', str(root / 'include'),
                            str(p / 'test.c'), '-o', str(p / 'test')], check=True,
                           capture_output=True, text=True)
            subprocess.run([str(p / 'test')], check=True)
