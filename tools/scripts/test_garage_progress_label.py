"""History labels: full native cursors, live globals and actual decimal helper."""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest


class ProgressLabelTest(unittest.TestCase):
    def test_live_counts_and_native_cursors(self):
        root = Path(__file__).resolve().parents[2]
        bodies = '\n'.join((root / f'src/overlays/PAL/garage/func_{a}.c').read_text()
                           for a in ('80117854',))
        formatter = (root / 'tools/scripts/fixtures/views/func_8002B814.c').read_text() + '\n' + (root / 'tools/scripts/fixtures/views/func_80042D28.c').read_text()
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
 RrUiUvRect uv[128];u16 counts[4],total;
 s32 tick;u32 guard1[4];
} State;
typedef struct {s32 op,x,y,a,b,c;size_t cursor;u8 data[8];} Event;
typedef struct {State s;Event events[12];unsigned count;} World;
static World actual,reference,initial,*w;
static unsigned numberSeen[81],totalSeen[321],clutSeen[2];
static u8 arena[1024];static u32 ordering[4];char FractionText[8];
RrBadgeRow1C *BadgeRows;
#define SaveProgress w->s.save
#define SelectionRows w->s.choice
#define UiFontGlyphs w->s.uv
#define ScreenFrame w->s.tick
#define garage_D_801272D8 w->s.counts
#define garage_D_801272FC w->s.total
s8 garage_D_8012621C[7]={23,24,-1,31,25,24,-2};
''' + bodies + formatter + r'''
static u32 next(void){u32 x=w->s.rng;x^=x<<13;x^=x>>17;x^=x<<5;return w->s.rng=x;}
static void change(void){
 unsigned i;
 BadgeRows=w->s.rows+(next()%4);
 w->s.choice.bytes[0]=next()%4;w->s.choice.bytes[1]=next()%4;w->s.choice.bytes[2]=next()%99;
 for(i=0;i<128;i++)w->s.uv[i].w=next();
 w->s.total=next()%321;
 for(i=0;i<4;i++)w->s.counts[i]=next()%81;
}
static Event *event(s32 op,void *cursor){
 Event *e;assert(w->count<12);e=&w->events[w->count++];memset(e,0,sizeof(*e));
 e->op=op;if(cursor){assert((uintptr_t)cursor>UINT32_MAX);e->cursor=(u8*)cursor-arena;assert(e->cursor<sizeof(arena));}
 return e;
}
s32 rsin(s32 angle){Event *e=event(1,0);s32 v=(s32)(next()%3)-1;e->a=angle;e->b=v;change();return v;}
void *AddSpritePrim(u32 *ot,void *p,s32 x,s32 y,const RrUiUvRect *uv,u16 clut,s32 color){
 Event *e=event(2,p);assert(ot==ordering);e->x=x;e->y=y;e->a=clut;e->b=color;memcpy(e->data,uv,4);change();return (u8*)p+20;
}
void *AddTextSprites(u32 *ot,void *p,s32 x,s32 y,const char *text,s32 clut,s32 spacing,s32 color){
 Event *e=event(3,p);size_t n=strlen(text);assert(n<8&&ot==ordering);if(text[2]=='/'){unsigned v=(text[0]-'0')*10+text[1]-'0';assert(v<=80);numberSeen[v]++;}
 else{unsigned v=(text[0]-'0')*100+(text[1]-'0')*10+text[2]-'0';assert(v<=320);totalSeen[v]++;}
 assert(clut==32760||clut==32761);clutSeen[clut-32760]++;
 e->x=x;e->y=y;e->a=clut;e->b=spacing;e->c=color;memcpy(e->data,text,n+1);change();return (u8*)p+20*n;
}
void *AddDrawModePrim(u32 *ot,void *p,u16 mode){Event *e=event(4,p);assert(ot==ordering);e->a=mode;change();return (u8*)p+12;}
void *AddTilePrim(u32 *ot,void *p,s32 x,s32 y,s32 a,s32 b,s32 r,s32 g,s32 blue){
 Event *e=event(5,p);assert(ot==ordering&&r==255&&g==255&&blue==255);e->x=x;e->y=y;e->a=a;e->b=b;change();return (u8*)p+24;
}
static void decimal(char *p,unsigned n,unsigned digits){while(digits){p[--digits]='0'+n%10;n/=10;}}
static void expected(u32 *ot,void *p,s32 row){
 int x=BadgeRows[row].x04+8,y=BadgeRows[row].y06+32,flag=0,i;
 for(i=0;i<4;i++)if(w->s.choice.bytes[0]==w->s.save.history6C[i].selection){flag=rsin((s32)(((u32)w->s.tick*128-682U)&4094U))<0;break;}
 for(i=0;i<7;i++){
  int key=garage_D_8012621C[i];
  if(key<0){
   unsigned digits=key==-1?2:3,n=key==-1?w->s.counts[w->s.choice.bytes[0]]:w->s.total;
   char text[8]={0};decimal(text,n,digits);text[digits]='/';decimal(text+digits+1,key==-1?80:320,digits);
   p=AddTextSprites(ot,p,x,y,text,32760+flag,0,0);
   if(key==-1){x+=34;p=AddTilePrim(ot,p,x,y-2,1,20,255,255,255);x+=8;}
  }else{
   p=AddSpritePrim(ot,p,x,y,&w->s.uv[key],32760,0);
   x+=w->s.uv[(unsigned)key].w+2;
  }
 }
 AddDrawModePrim(ot,p,56);
}
int main(void){unsigned scenario,i;
 assert((uintptr_t)arena>UINT32_MAX);_Static_assert(sizeof(RrSaveProgressRecord7C)==124,"full save");
 for(scenario=0;scenario<65536;scenario++){
  memset(&initial,0,sizeof(initial));w=&initial;w->s.rng=0x17854+scenario*17;change();
  for(i=0;i<4;i++){
   w->s.rows[i].x04=(s16)next();w->s.rows[i].y06=(s16)next();
   w->s.save.history6C[i].selection=next()%4;w->s.save.history6C[i].variant=next()%4;w->s.save.history6C[i].parameter=next()%99;
  }
  if(scenario%5<4){RrSelectionRow *h=&w->s.save.history6C[scenario%5];h->selection=w->s.choice.bytes[0];h->variant=w->s.choice.bytes[1];h->parameter=w->s.choice.bytes[2];}
  w->s.tick=(s32)next();actual=reference=initial;
  w=&actual;BadgeRows=w->s.rows;garage_func_80117854(ordering,arena,scenario%4);
  w=&reference;BadgeRows=w->s.rows;expected(ordering,arena,scenario%4);
  assert(!memcmp(&actual,&reference,sizeof(actual)));
 }
 for(i=0;i<81;i++)assert(numberSeen[i]);for(i=0;i<321;i++)assert(totalSeen[i]);assert(clutSeen[0]&&clutSeen[1]);
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
