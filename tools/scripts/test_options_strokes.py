"""Five live path owners, wrapping arithmetic and mutable line-emitter protocol.

The line emitter is simulated; this does not execute GPU hardware.
"""
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess, unittest
ROOT=Path(__file__).resolve().parents[2]
class StrokePathsTest(unittest.TestCase):
 def test_all_halfword_factors_and_mutable_owners(self):
  body=(ROOT/'src/overlays/PAL/options/func_80116AA4.c').read_text().split('void options_draw_button_guides',1)[1]
  code=r'''
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "rr/overlay665_strokes.h"
typedef struct {u32 guard[4];RrStrokeStep4 banks[10][9];u8 selected[5];RrStrokeColor4 color;u16 origins[5][2];u32 rng,tail[4];} State;
typedef struct {State before;u16 xy[4];u8 rgba[4];s32 blend,depth;} Event;
typedef struct {State s;const RrStrokeStep4 *paths[5];Event events[80];unsigned count;} World;
static World actual,reference,*w;
static unsigned scenario,mutation,seen[16],directions[4],factorSeen[65536],lengthSeen[65536],originSeen[5];
static const s32 factors[12]={INT32_MIN,INT32_MIN+1,INT32_MAX-1,INT32_MAX,-65537,-65536,-17,-1,0,1,16,65536};
#define options_button_guide_line_color (w->s.color)
#define options_button_guide_origins (w->s.origins)
static s32 word(u32 n){s32 v;memcpy(&v,&n,4);return v;}
static s16 half(u16 n){s16 v;memcpy(&v,&n,2);return v;}
static u32 next(void){u32 n=w->s.rng;n^=n<<13;n^=n>>17;n^=n<<5;return w->s.rng=n;}
static void sync(void){unsigned i;for(i=0;i<5;i++){assert(w->s.selected[i]<10);w->paths[i]=w->s.banks[w->s.selected[i]];}}
void AddLineF2Prim(const s16 *points,const u8 *color,s32 blend,s32 depth){u16 *xy=(u16*)points;u8 *rgb=(u8*)color;Event *e;unsigned i,slot,bank,row;assert(w->count<80);assert(blend==0&&depth==0);
 e=&w->events[w->count++];memset(e,0,sizeof(*e));e->before=w->s;memcpy(e->xy,xy,8);memcpy(e->rgba,rgb,4);e->blend=blend;e->depth=depth;
 if(mutation){
  for(i=0;i<4;i++){xy[i]=(u16)next();rgb[i]=(u8)next();w->s.color.bytes[i]=(u8)next();}
  slot=next()%5;w->s.selected[slot]=next()%10;w->s.origins[slot][0]=(u16)next();w->s.origins[slot][1]=(u16)next();
  bank=next()%10;row=next()%8;w->s.banks[bank][row].direction=next()%4;w->s.banks[bank][row].length=half((u16)next());sync();
 }
}
void options_draw_button_guides'''+body+r'''
static void expected(s32 factor){RrStrokeColor4 color=w->s.color;unsigned path,row,bank;u16 xy[4];s32 total,limit,used,n,nextUsed,dir;RrStrokeStep4 step;
 if(!factor){seen[0]++;return;}seen[1+(factor<0)]++;
 for(path=0;path<5;path++){
  originSeen[path]++;xy[0]=w->s.origins[path][0];xy[1]=w->s.origins[path][1];bank=w->s.selected[path];total=0;
  for(row=0;row<9&&w->s.banks[bank][row].direction!=-1;row++)total=word((u32)total+(u32)(s32)w->s.banks[bank][row].length);
  assert(row<9);limit=word((u32)total*(u32)factor)/16;used=0;bank=w->s.selected[path];
  for(row=0;row<9&&w->s.banks[bank][row].direction!=-1;row++){
   step=w->s.banks[bank][row];dir=step.direction;assert(dir>=0&&dir<4);directions[dir]++;lengthSeen[(u16)step.length]++;
   nextUsed=word((u32)used+(u32)(s32)step.length);
   if(limit>=nextUsed){n=step.length;used=nextUsed;seen[3]++;}else{n=word((u32)limit-(u32)used);used=word((u32)used+(u32)n);seen[4]++;}
   xy[2]=xy[0];xy[3]=xy[1];
   if(dir<2){xy[3]=(u16)((u32)xy[1]+(dir?(u32)n:0U-(u32)n));AddLineF2Prim((const s16 *)xy,color.bytes,0,0);seen[5]++;}
   else{xy[2]=(u16)((u32)xy[0]+(dir==3?(u32)n:0U-(u32)n));AddLineF2Prim((const s16 *)xy,color.bytes,0,0);xy[1]=(u16)(xy[1]+1U);xy[3]=(u16)(xy[3]+1U);AddLineF2Prim((const s16 *)xy,color.bytes,0,0);seen[6]++;}
   xy[0]=xy[2];xy[1]=xy[3];
  }
  assert(row<9);if(!row)seen[7]++;
 }
}
int main(void){unsigned i,j,k;u32 guard[4];s32 factor;uint64_t cases=0;
 _Static_assert(sizeof(RrStrokeStep4)==4,"complete step");_Static_assert(sizeof(RrStrokeColor4)==4,"complete color");
 assert((uintptr_t)&actual.s.banks[0][0]>UINT32_MAX);
 for(scenario=0;scenario<73728;scenario++)for(mutation=0;mutation<2;mutation++){
  memset(&actual,0,sizeof(actual));memset(&reference,0,sizeof(reference));w=&actual;w->s.rng=0x16AA4U+scenario*73U;
  for(i=0;i<10;i++){for(j=0;j<8;j++){w->s.banks[i][j].direction=next()%4;w->s.banks[i][j].length=half((u16)next());}w->s.banks[i][8].direction=-1;w->s.banks[i][8].length=half((u16)next());}
  for(i=0;i<5;i++){w->s.selected[i]=next()%10;w->s.origins[i][0]=(u16)next();w->s.origins[i][1]=(u16)next();}
  if(scenario%7==0)w->s.banks[w->s.selected[scenario%5]][scenario%9].direction=-1;
  for(i=0;i<4;i++){w->s.color.bytes[i]=(u8)next();w->s.guard[i]=next();w->s.tail[i]=next();guard[i]=w->s.guard[i];}
  factor=scenario<65536?half((u16)scenario):(scenario%2?factors[(scenario>>1)%12]:word(next()));if(scenario<65536)factorSeen[(u16)factor]++;
  reference.s=actual.s;w=&actual;sync();options_draw_button_guides(w->paths,factor);w=&reference;sync();expected(factor);
  assert(!memcmp(&actual.s,&reference.s,sizeof(State)));assert(actual.count==reference.count);assert(!memcmp(actual.events,reference.events,actual.count*sizeof(Event)));
  for(k=0;k<5;k++){assert(actual.paths[k]==actual.s.banks[actual.s.selected[k]]);assert(reference.paths[k]==reference.s.banks[reference.s.selected[k]]);}assert(!memcmp(actual.s.guard,guard,sizeof(guard)));cases++;
 }
 assert(cases==147456);for(i=0;i<65536;i++){assert(factorSeen[i]);assert(lengthSeen[i]);}for(i=0;i<8;i++)assert(seen[i]);for(i=0;i<4;i++)assert(directions[i]);for(i=0;i<5;i++)assert(originSeen[i]);return 0;
}
'''
  with TemporaryDirectory() as tmp:
   p=Path(tmp);(p/'test.c').write_text(code)
   r=subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(ROOT/'include'),str(p/'test.c'),'-o',str(p/'test')],capture_output=True,text=True)
   self.assertEqual(r.returncode,0,r.stderr);subprocess.run([str(p/'test')],check=True)
if __name__=='__main__':unittest.main()
