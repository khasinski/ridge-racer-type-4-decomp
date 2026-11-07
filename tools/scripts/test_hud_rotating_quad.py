"""Actual HUD quad emitter and trig bodies, with guarded full native owners."""
from pathlib import Path
from tempfile import TemporaryDirectory
import math,re,struct,subprocess,unittest
ROOT=Path(__file__).resolve().parents[2]
class HudRotatingQuadTest(unittest.TestCase):
 def test_phase_wrapping_native_roots_and_overlapping_tag(self):
  body=(ROOT/'tools/scripts/fixtures/legacy/func_80021350.c').read_text()
  body=re.sub(r'^#include[^\n]*\n','',body,flags=re.M)
  body=re.sub(r'\s+__asm__\("\$\d+"\)','',body)
  body=re.sub(r'__asm__\(""[^;]*;','',body)
  helpers=''
  for name in ['trig_sine.c','func_800904F4.c']:
   text=(ROOT/'tools/scripts/fixtures/legacy'/name).read_text();helpers+=re.sub(r'^#include[^\n]*\n','',text,flags=re.M)+'\n'
  for old,new in [('rsin','real_sin'),('sin_1','real_lookup'),('rcos','real_cos')]:helpers=helpers.replace(old,new)
  exe=(ROOT/'assets/PAL/main.exe').read_bytes();start=0x800+0x800A50C0-0x80010000
  rom=struct.unpack('<4097h',exe[start:start+8194]);sine=[round(4096*math.sin(i*math.tau/4096)) for i in range(4096)]
  code=r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4||sizeof(t)==n,"PAL size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4||offsetof(t,f)==n,"PAL offset")
#include "rr/hud_rotating_quad.h"
#include "rr/trig.h"
'''+ 'static const s16 rom[4097]={'+','.join(map(str,rom))+'};\nstatic const s32 sineTable[4096]={'+','.join(map(str,sine))+'};\n'+r'''
#define D_800A50C0 rom
#define D_800A58C0 (rom+0x400)
#define D_800A60C0 (rom+0x800)
#define SinTable (rom+0xC00)
s32 real_lookup(s32);
'''+helpers+r'''
enum {ARENA=544,GUARD=16,ROWS=4};
typedef struct {u32 before[4];RrNumberTarget targets[ROWS];u32 heads[ROWS][4];
 RrGpuScratchPointers scratch;RrHudQuadCorner corners[4];s32 bank;u32 after[4];} Environment;
static Environment actual,expected,pre[2];
static u8 *arena,reference[ARENA],preArena[2][ARENA];
static unsigned calls,profile,scenario,rowIndex,phaseHits[4096],selections[4][4],aliases[2];
static s32 wantedAngle,results[2],stress[2];static size_t emitted;
static u32 seed=0x21350;
static u32 next(void){u32 x=seed;x^=x<<13;x^=x>>17;x^=x<<5;return seed=x;}
static s32 sign32(u32 x){s32 y;memcpy(&y,&x,4);return y;}
static s16 sign16(u16 x){s16 y;memcpy(&y,&x,2);return y;}
static s8 sign8(u8 x){s8 y;memcpy(&y,&x,1);return y;}
static size_t cursor_offset(u8 *p){assert(p>=arena&&p+40<=arena+ARENA-GUARD);return (size_t)(p-arena);}
static void mutate(Environment *e,u8 *bytes,unsigned id){unsigned r,b;size_t offset;
 if(profile){
  e->bank=(s32)((scenario+id)&3);offset=GUARD+4*((scenario+id*7)%110);
  e->scratch.cursor=arena+offset;
  for(r=0;r<ROWS;r++){
   e->targets[r].x=sign16((u16)(scenario+r*31+id));
   e->targets[r].y=sign16((u16)(scenario*17+r+id));
   for(b=0;b<4;b++){
    e->heads[r][b]^=0x81234567U+id+r+b;
    e->targets[r].heads[b]=&actual.heads[(r+id)&3][(b+1)&3];
   }
  }
  for(r=0;r<4;r++){e->corners[r].x=sign8((u8)(scenario+r+id));e->corners[r].y=sign8((u8)((scenario>>8)+r-id));}
  bytes[offset]^=0x96;bytes[offset+3]^=0x5a;
 }
 if(profile==2&&(scenario&1))e->targets[rowIndex].heads[e->bank]=(u32 *)e->scratch.cursor;
}
static s32 math_call(s32 angle,unsigned id){s32 value;
 assert(calls==id&&angle==wantedAngle);assert(!memcmp(&actual,&pre[id],sizeof(actual)));
 assert(!memcmp(arena,preArena[id],ARENA));
 value=id?real_cos(angle):real_sin(angle);assert(value==results[id]);
 mutate(&actual,arena,id);calls++;
 return profile==3?stress[id]:value;
}
static s32 call_sin(s32 angle){return math_call(angle,0);}
static s32 call_cos(s32 angle){return math_call(angle,1);}
#define HudNumberTargets (actual.targets)
#define DrawBufIndex (actual.bank)
#define HudQuadCorners (actual.corners)
#undef RR_HUD_QUAD_SCRATCH
#define RR_HUD_QUAD_SCRATCH (&actual.scratch)
#define rsin call_sin
#define rcos call_cos
'''+body+r'''
static void put16(size_t offset,u16 value){memcpy(reference+offset,&value,2);}
static void put32(size_t offset,u32 value){memcpy(reference+offset,&value,4);}
static u32 get32(size_t offset){u32 value;memcpy(&value,reference+offset,4);return value;}
static u32 head_read(u32 *head){unsigned r,b;
 for(r=0;r<ROWS;r++)for(b=0;b<4;b++)if(head==&actual.heads[r][b])return expected.heads[r][b];
 assert(head==(u32 *)(arena+emitted));return get32(emitted);
}
static void head_write(u32 *head,u32 value){unsigned r,b;
 for(r=0;r<ROWS;r++)for(b=0;b<4;b++)if(head==&actual.heads[r][b]){expected.heads[r][b]=value;return;}
 assert(head==(u32 *)(arena+emitted));put32(emitted,value);
}
static void model(s32 phase){s32 scaled;int64_t x,y,dx,dy;s32 sn,cs;u32 *head;unsigned i,bank;
 static const u8 colors[4][3]={{208,24,48},{104,12,24},{138,18,36},{52,6,12}};
 scaled=sign32((u32)((uint64_t)(u32)phase*2560U));
 wantedAngle=(s32)((int64_t)scaled/10000)+3584;
 phaseHits[(u32)wantedAngle&4095U]++;
 results[0]=sineTable[(u32)wantedAngle&4095U];results[1]=sineTable[((u32)wantedAngle+1024U)&4095U];
 for(i=0;i<2;i++){pre[i]=expected;memcpy(preArena[i],reference,ARENA);mutate(&expected,reference,i);}
 sn=profile==3?stress[0]:results[0];cs=profile==3?stress[1]:results[1];
 bank=(unsigned)expected.bank;assert(bank<4);selections[rowIndex][bank]++;
 x=(u16)expected.targets[rowIndex].x;y=(u16)expected.targets[rowIndex].y;
 head=expected.targets[rowIndex].heads[bank];emitted=cursor_offset(expected.scratch.cursor);
 reference[emitted+4]=176;reference[emitted+5]=176;reference[emitted+6]=0;
 for(i=0;i<4;i++){
  dx=sign32((u32)((int64_t)cs*expected.corners[i].x-(int64_t)sn*expected.corners[i].y));
  dy=sign32((u32)((int64_t)sn*expected.corners[i].x+(int64_t)cs*expected.corners[i].y));
  put16(emitted+8+i*8,(u16)(x+dx/4096));put16(emitted+10+i*8,(u16)(y+dy/4096));
  memcpy(reference+emitted+12+i*8,colors[i],3);
 }
 reference[emitted+3]=8;reference[emitted+7]=56;
 put32(emitted,(get32(emitted)&0xff000000U)|(head_read(head)&0xffffffU));
 head_write(head,(head_read(head)&0xff000000U)|((uintptr_t)(arena+emitted)&0xffffffU));
 aliases[head==(u32 *)(arena+emitted)]++;
 expected.scratch.cursor=arena+emitted+36;
}
int main(void){unsigned raw,r,b,i;uint64_t cases=0;s32 phase;
 assert(sizeof(void*)==8&&sizeof(unsigned long)==sizeof(void*));
 assert(sizeof(RrHudQuadColor)==4&&sizeof(RrHudQuadEmission)==40);
 assert(sizeof(RrNumberTarget)==40&&offsetof(RrNumberTarget,x)==32);
 arena=malloc(ARENA);assert(arena&&((uintptr_t)arena>>32)&&((uintptr_t)&actual.targets>>32));
 for(raw=0;raw<65536;raw++)for(profile=0;profile<4;profile++){
  u8 *env=(u8 *)&actual;scenario=raw;rowIndex=(raw>>2)&3;
  for(i=0;i<sizeof(actual);i++)env[i]=(u8)(i*37U+raw+profile);
  for(i=0;i<ARENA;i++)arena[i]=(u8)(i*19U+raw+profile);
  for(r=0;r<ROWS;r++){
   actual.targets[r].x=sign16((u16)raw);actual.targets[r].y=sign16((u16)(raw*17U+profile));
   for(b=0;b<4;b++){actual.targets[r].heads[b]=&actual.heads[r][b];actual.heads[r][b]=next();}
  }
  for(i=0;i<4;i++){actual.corners[i].x=sign8((u8)(raw+i*17));actual.corners[i].y=sign8((u8)((raw>>8)+i*31));}
  actual.bank=(s32)(raw&3);actual.scratch.cursor=arena+GUARD+4*(raw%110);
  actual.scratch.order=&actual.heads[0][0];
  phase=sign32((u32)raw|(profile==1?0x400000U:profile==2?0x80000000U:profile==3?(next()&0xffff0000U):0));
  stress[0]=sign32(next());stress[1]=sign32(next());
  expected=actual;memcpy(reference,arena,ARENA);calls=0;model(phase);
  DrawHudRotatingQuad(phase,(s32)rowIndex);assert(calls==2);
  assert(!memcmp(&actual,&expected,sizeof(actual)));
  /* Only these four genuine union-padding bytes are unspecified. The RGB
   * writes, including the next tag's three bytes at36, are all compared. */
  for(i=0;i<4;i++){arena[emitted+15+i*8]=0;reference[emitted+15+i*8]=0;}
  assert(!memcmp(arena,reference,ARENA));cases++;
 }
 for(r=0;r<ROWS;r++)for(b=0;b<4;b++)assert(selections[r][b]);
 for(i=0;i<4096;i++)assert(phaseHits[i]);assert(aliases[0]&&aliases[1]);free(arena);
 printf("%llu cases; actual trig ROM/bodies, all phases, native roots, tag alias and four RGB tail writes\n",(unsigned long long)cases);
}
'''
  with TemporaryDirectory() as temp:
   path=Path(temp);(path/'test.c').write_text(code)
   subprocess.run(['cc','-std=c11','-O2','-Wall','-Wextra','-Werror','-Wno-error=deprecated-non-prototype','-Wno-unused-function','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(ROOT/'include'),str(path/'test.c'),'-o',str(path/'test')],check=True)
   subprocess.run([str(path/'test')],check=True)
if __name__=='__main__':unittest.main()
