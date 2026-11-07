"""Character grid with live selection/UV state and full native packet cursors."""
from pathlib import Path
from tempfile import TemporaryDirectory
import re
import subprocess
import unittest


class CharacterGridTest(unittest.TestCase):
    def test_captured_coordinates_and_live_selection(self):
        root = Path(__file__).resolve().parents[2]
        body = (root / 'src/overlays/PAL/garage/func_80117F54.c').read_text()
        body = re.sub(r' __asm__\("\$[0-9]+"\)', '', body)
        source = r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4||sizeof(t)==n,"PAL size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4||offsetof(t,f)==n,"PAL offset")
#include "rr/overlay662_character_grid.h"
typedef struct {u32 guard[4],rng;RrBadgeRow1C rows[4];RrUiUvRect uv[128];s32 selection,tick;u32 tail[4];} State;
typedef struct {s32 op,args[9];size_t cursor;u32 hash;u8 text[4];} Event;
typedef struct {State s;Event e[128];unsigned count;} World;
static World actual,reference,initial,*w;
static u8 arena[2048];static u32 ordering[4];static unsigned scenario,mutation;
static unsigned selectedSeen[43],clutSeen[2],sineSeen[9];
static const s32 sineValues[9]={INT32_MIN,-4096,-1,0,1,63,64,4096,INT32_MAX};
u8 NameCharset[]=" 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ!?&@";
s8 garage_name_entry_button_glyphs[3]={0,1,2};
RrBadgeRow1C *BadgeRows;
#define UiFontGlyphs w->s.uv
#define ScreenFrame w->s.tick
#define garage_name_key_index w->s.selection
static s32 word(u32 n){s32 v;memcpy(&v,&n,4);return v;}
static u32 next(void){u32 x=w->s.rng;x^=x<<13;x^=x>>17;x^=x<<5;return w->s.rng=x;}
static void change(void){unsigned i;if(!mutation)return;
 BadgeRows=w->s.rows+(next()%4);w->s.selection=(s32)((scenario+w->count)%45)-1;w->s.tick=word(next());
 for(i=0;i<128;i++)w->s.uv[i].w=(u8)next();
}
static Event *event(s32 op,void *cursor){Event *e;unsigned i;u32 h=2166136261U;
 assert(w->count<128);e=&w->e[w->count++];memset(e,0,sizeof(*e));e->op=op;
 for(i=0;i<sizeof(w->s);i++)h=(h^((u8*)&w->s)[i])*16777619U;e->hash=h;
 if(cursor){assert((uintptr_t)cursor>UINT32_MAX);e->cursor=(u8*)cursor-arena;assert(e->cursor<sizeof(arena));}return e;
}
s32 rcos(s32 angle){Event *e=event(1,0);unsigned n=(scenario+w->count)%9;s32 v=sineValues[n];
 e->args[0]=angle;e->args[1]=v;sineSeen[n]++;change();return v;
}
void *AddTextSprites(u32 *ot,void *p,s32 x,s32 y,const char *text,s32 clut,s32 spacing,s32 color){
 Event *e=event(2,p);assert(ot==ordering&&text[0]&&text[1]==0&&text[2]==0&&text[3]==0);
 assert(clut==32760||clut==32761);clutSeen[clut-32760]++;
 e->args[0]=x;e->args[1]=y;e->args[2]=clut;e->args[3]=spacing;e->args[4]=color;memcpy(e->text,text,4);change();return (u8*)p+20;
}
void *AddSpritePrim(u32 *ot,void *p,s32 x,s32 y,const RrUiUvRect *uv,u16 clut,s32 color){
 Event *e=event(3,p);assert(ot==ordering&&uv>=w->s.uv&&uv<w->s.uv+128);assert(clut==32760||clut==32761);clutSeen[clut-32760]++;
 e->args[0]=x;e->args[1]=y;e->args[2]=clut;e->args[3]=color;e->args[4]=uv-w->s.uv;memcpy(e->text,uv,4);change();return (u8*)p+20;
}
void *AddTilePrim(u32 *ot,void *p,s32 x,s32 y,s32 a,s32 b,s32 r,s32 g,s32 blue){
 Event *e=event(4,p);assert(ot==ordering);e->args[0]=x;e->args[1]=y;e->args[2]=a;e->args[3]=b;e->args[4]=r;e->args[5]=g;e->args[6]=blue;change();return (u8*)p+24;
}
void *AddDrawModePrim(u32 *ot,void *p,u16 mode){Event *e=event(5,p);assert(ot==ordering);e->args[0]=mode;change();return (u8*)p+12;}
''' + body + r'''
static void pulse(unsigned selected,s32 *clut,s32 *color){
 *clut=32760;*color=0;
 if(selected){s32 v=rcos(word((u32)w->s.tick*128U-682U));
  if(v<0)*clut=32761;else *color=(s32)(((int64_t)v/64+100)*2);
 }
}
static void expected(s32 row){s32 x=w->s.rows[row].x04+36,y=w->s.rows[row].y06+56,clut,color;unsigned i,dx=0;void *p=arena;
 for(i=0;i<40;i++){
  char text[4]={(char)NameCharset[i+1],0,0,0};pulse((u32)w->s.selection==i,&clut,&color);
  p=AddTextSprites(ordering,p,x+(i%10)*24,y+(i/10)*32,text,clut,0,color);
 }
 p=AddTilePrim(ordering,p,x-16,y+136,256,2,255,255,255);
 for(i=0;i<4;i++){
  p=AddTilePrim(ordering,p,x-12+dx,y+144,1,16,255,255,255);dx+=4;
  if(i!=3){unsigned key=garage_name_entry_button_glyphs[i];pulse((u32)w->s.selection==40+i,&clut,&color);
   p=AddSpritePrim(ordering,p,x-12+dx,y+144,&w->s.uv[key],clut,color);dx+=4+w->s.uv[key].w;
  }
 }
 AddDrawModePrim(ordering,p,56);
}
int main(void){unsigned i,j;uint64_t cases=0;
 assert(sizeof(RrCharacterGridAddress)==sizeof(void*)&&(uintptr_t)arena>UINT32_MAX);
 for(scenario=0;scenario<32768;scenario++)for(mutation=0;mutation<2;mutation++){
  memset(&initial,0,sizeof(initial));w=&initial;w->s.rng=0x17F54+scenario*17;
  for(i=0;i<4;i++){w->s.guard[i]=next();w->s.tail[i]=next();w->s.rows[i].x04=(s16)next();w->s.rows[i].y06=(s16)next();}
  for(i=0;i<128;i++){w->s.uv[i].u=(u8)next();w->s.uv[i].v=(u8)next();w->s.uv[i].w=(u8)next();w->s.uv[i].h=(u8)next();}
  w->s.tick=word(next());w->s.selection=(s32)(scenario%50)-3;
  if(scenario%64==0)w->s.selection=INT32_MIN;else if(scenario%64==1)w->s.selection=INT32_MAX;
  if((u32)w->s.selection<43)selectedSeen[w->s.selection]++;
  actual=reference=initial;w=&actual;BadgeRows=w->s.rows;garage_draw_name_char_grid(ordering,arena,scenario%4);
  w=&reference;BadgeRows=w->s.rows;expected(scenario%4);
  assert(!memcmp(&actual,&reference,sizeof(actual)));cases++;
 }
 assert(cases==65536);for(i=0;i<43;i++)assert(selectedSeen[i]);for(i=0;i<9;i++)assert(sineSeen[i]);assert(clutSeen[0]&&clutSeen[1]);return 0;
}
'''
        with TemporaryDirectory() as tmp:
            p = Path(tmp)
            (p / 'test.c').write_text(source)
            subprocess.run(['cc', '-std=gnu11', '-O2', '-fsanitize=undefined',
                            '-fno-sanitize-recover=all', '-I', str(root / 'include'),
                            str(p / 'test.c'), '-o', str(p / 'test')], check=True)
            subprocess.run([str(p / 'test')], check=True)
