"""SPU reset against a complete-bank model and simulated polling/services.

The native-only status accessor supplies deterministic device-ready sequences.
No real SPU, DMA, interrupt, or asynchronous hardware is executed.
"""
from pathlib import Path
from tempfile import TemporaryDirectory
import re, subprocess, unittest
ROOT=Path(__file__).resolve().parents[2]

class SpuInitializationTest(unittest.TestCase):
 def test_all_status_words_modes_and_poll_boundaries(self):
  body=(ROOT/'tools/scripts/fixtures/legacy/func_8007EAC4.c').read_text()
  body=re.sub(r'^#include[^\n]*\n','',body,flags=re.M)
  body=re.sub(r' __asm__\("\$\d+"\)','',body)
  body=re.sub(r'__asm__\(""[^;]*;',';',body)
  body=body.replace('#define STATUS(bank) ((bank)->halves[215])','#ifdef DIRECT_STATUS\n#define STATUS(bank) ((bank)->halves[215])\n#else\n#define STATUS(bank) status_read(bank)\n#endif')
  code=r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>
#include "rr/spu_init.h"
typedef struct {u32 guard[4];RrSpuRegisterStorage bank;u32 tail[4];} Bank;
typedef struct {
 u16 banks[2][256],entries[10],transfer;
 u32 guards[2][4],tails[2][4],entryGuard[4],entryTail[4],dma[2];
 s32 f4,f8,fc,mode,active;u32 shift,rounding,unknown,root,dmaRoot;
 unsigned callback,reads;
} Snapshot;
typedef struct {Snapshot snapshot;u32 service,arg0,arg1;} Event;
typedef struct {
 Bank banks[2];
 struct {u32 guard[4];u16 values[10];u32 tail[4];} entries;
 volatile u32 dma[2];volatile u16 transfer;
 s32 f4,f8,fc,mode,active;u32 shift,rounding;volatile u32 unknown;
 void (*volatile callback)(void);
 RrSpuRegisterStorage *root;volatile u32 *dmaRoot;
 u32 seed,phase,plan,reads,count;Event events[16];
} Environment;
static Environment actual,expected,*g;
const u16 SpuInitFillData[8]={0x707,0x707,0x707,0x707,0x707,0x707,0x707,0x707};
const char SpuTimeoutFmt[]="timeout",SpuWaitResetStr[]="reset";
#define _spu_RXX (&g->root->registers)
#define SPU_DPCR (g->dmaRoot)
#define _spu_tsa (g->transfer)
#define _spu_transMode (g->f4)
#define _spu_addrMode (g->f8)
#define _spu_mem_mode (g->fc)
#define _spu_mem_mode_plus (g->mode)
#define _spu_mem_mode_unit (g->shift)
#define _spu_mem_mode_unitM (g->rounding)
#define _spu_inTransfer (g->active)
#define _spu_transferCallback (g->callback)
#define _spu_IRQCallback (g->unknown)
#define _spu_RQ (g->entries.values)
static u32 next(u32 *p){u32 n=*p;n^=n<<13;n^=n>>17;n^=n<<5;return *p=n;}
static void callback(void){}
static void snapshot(Snapshot *s){unsigned i,j;memset(s,0,sizeof(*s));
 for(i=0;i<2;i++){for(j=0;j<256;j++)s->banks[i][j]=g->banks[i].bank.halves[j];for(j=0;j<4;j++){s->guards[i][j]=g->banks[i].guard[j];s->tails[i][j]=g->banks[i].tail[j];}s->dma[i]=g->dma[i];}
 for(i=0;i<10;i++)s->entries[i]=g->entries.values[i];for(i=0;i<4;i++){s->entryGuard[i]=g->entries.guard[i];s->entryTail[i]=g->entries.tail[i];}
 s->transfer=g->transfer;s->f4=g->f4;s->f8=g->f8;s->fc=g->fc;s->mode=g->mode;s->active=g->active;s->shift=g->shift;s->rounding=g->rounding;s->unknown=g->unknown;s->reads=g->reads;
 assert(g->root==&g->banks[0].bank||g->root==&g->banks[1].bank);s->root=g->root==&g->banks[1].bank;
 assert(g->dmaRoot==&g->dma[0]||g->dmaRoot==&g->dma[1]);s->dmaRoot=g->dmaRoot==&g->dma[1];
 assert(!g->callback||g->callback==callback);s->callback=g->callback!=0;
}
static void event(u32 service,u32 arg0,u32 arg1){Event *e;assert(g->count<16);e=&g->events[g->count++];e->service=service;e->arg0=arg0;e->arg1=arg1;snapshot(&e->snapshot);}
static void mutate(u32 service){u32 r,i;if(!g->phase)return;r=next(&g->seed);
 g->root=&g->banks[r&1].bank;g->dmaRoot=&g->dma[(r>>1)&1];
 for(i=0;i<8;i++)g->root->halves[(next(&g->seed)%256)]=(u16)next(&g->seed);
 g->f4=(s32)next(&g->seed);g->f8=(s32)next(&g->seed);g->fc=(s32)next(&g->seed);g->mode=(s32)next(&g->seed);g->active=(s32)next(&g->seed);g->shift=next(&g->seed);g->rounding=next(&g->seed);g->unknown=next(&g->seed);g->transfer=(u16)next(&g->seed);
 g->callback=(r&2)?callback:0;g->entries.values[r%10]=(u16)(r^service);
}
void _spu_Fw1ts(void){event(1,0,0);mutate(1);}
void _spu_FwriteByIO(const u16 *p,u32 n){assert(p==SpuInitFillData&&n==16);for(unsigned i=0;i<8;i++)assert(p[i]==0x707);event(2,n,0);mutate(2);}
s32 DebugPrintStub(const char *f,...){va_list ap;const char *reason;va_start(ap,f);reason=va_arg(ap,const char*);va_end(ap);assert(f==SpuTimeoutFmt&&reason==SpuWaitResetStr);event(3,0,0);mutate(3);return -123;}
static u16 status_read(RrSpuRegisterStorage *bank){
#ifdef DIRECT_STATUS
 return bank->halves[215];
#else
 static const u32 ready[7]={0,1,2,3839,3840,3841,0xFFFFFFFFU};u32 index=g->reads++;u16 v=bank->halves[215];
 if(g->plan){if(index>=ready[g->plan-1])v&=(u16)~0x7FF;else v=(v&0xF800)|1;bank->halves[215]=v;}
 return v;
#endif
}
'''+body+r'''
static s32 model(s32 warm){u32 i,attempt;RrSpuRegisterStorage *b;
 *g->dmaRoot|=0xB0000;g->f4=0;g->f8=0;g->transfer=0;
 b=g->root;b->halves[192]=0;b->halves[193]=0;b->halves[213]=0;
 _spu_Fw1ts();b=g->root;b->halves[192]=0;b->halves[193]=0;
 if(status_read(b)&2047){for(attempt=0;attempt<3840;attempt++)if(!(status_read(g->root)&2047))break;if(attempt==3840)DebugPrintStub(SpuTimeoutFmt,SpuWaitResetStr);}
 g->fc=2;g->mode=3;g->shift=8;g->rounding=7;
 b=g->root;b->halves[214]=4;b->halves[194]=0;b->halves[195]=0;b->halves[198]=65535;b->halves[199]=65535;b->halves[204]=0;b->halves[205]=0;
 for(i=0;i<10;i++)g->entries.values[i]=0;
 if(!warm){
  g->transfer=512;b=g->root;
  for(i=200;i<204;i++)b->halves[i]=0;for(i=216;i<220;i++)b->halves[i]=0;
  _spu_FwriteByIO(SpuInitFillData,16);b=g->root;
  for(i=0;i<24;i++){b->halves[8*i]=0;b->halves[8*i+1]=0;b->halves[8*i+2]=16383;b->halves[8*i+3]=512;b->halves[8*i+4]=0;b->halves[8*i+5]=0;}
  b=g->root;b->halves[196]=65535;b->halves[197]=255;
  for(i=0;i<4;i++)_spu_Fw1ts();
  b=g->root;b->halves[198]=65535;b->halves[199]=255;
  for(i=0;i<4;i++)_spu_Fw1ts();
 }
 b=g->root;g->active=1;b->halves[213]=49152;g->callback=0;g->unknown=0;return 0;
}
static void initialize(Environment *e,u32 status,u32 phase,u32 plan,u32 *rng){unsigned i,j;
 memset(e,0,sizeof(*e));e->seed=next(rng);e->phase=phase;e->plan=plan;
 for(i=0;i<2;i++){for(j=0;j<256;j++)e->banks[i].bank.halves[j]=(u16)next(rng);e->banks[i].bank.halves[215]=(u16)status;for(j=0;j<4;j++){e->banks[i].guard[j]=next(rng);e->banks[i].tail[j]=next(rng);}e->dma[i]=next(rng);}
 for(i=0;i<10;i++)e->entries.values[i]=(u16)next(rng);for(i=0;i<4;i++){e->entries.guard[i]=next(rng);e->entries.tail[i]=next(rng);}
 e->transfer=(u16)next(rng);e->f4=(s32)next(rng);e->f8=(s32)next(rng);e->fc=(s32)next(rng);e->mode=(s32)next(rng);e->active=(s32)next(rng);e->shift=next(rng);e->rounding=next(rng);e->unknown=next(rng);e->callback=callback;e->root=&e->banks[status&1].bank;e->dmaRoot=&e->dma[(status>>1)&1];
}
int main(void){u32 n,phase,seed=0x7EAC4,planSeen[8]={0},modeSeen[2]={0},timeoutSeen[2]={0};Snapshot a,b;s32 warm;unsigned i;uint64_t cases=0;
 _Static_assert(sizeof(RrSpuRegisterStorage)==512,"complete bank");_Static_assert(sizeof(SpuInitFillData)==16,"bounded input");_Static_assert(sizeof(actual.entries.values)==20,"complete entry owner");
 assert((uintptr_t)&actual>UINT32_MAX&&(uintptr_t)&actual.banks[1]>UINT32_MAX);
 for(n=0;n<65536;n++)for(phase=0;phase<4;phase++){
  u32 plan=(n+phase)%8,initialSeed;
  warm=(phase&1)?(s32)(next(&seed)|1U):0;
  initialSeed=seed;
  initialize(&actual,n,phase>>1,plan,&seed);
  initialize(&expected,n,phase>>1,plan,&initialSeed);
  assert(seed==initialSeed);
  g=&expected;assert(model(warm)==0);snapshot(&b);g=&actual;assert(_spu_init(warm)==0);snapshot(&a);
  assert(!memcmp(&a,&b,sizeof(a)));assert(actual.seed==expected.seed&&actual.reads==expected.reads&&actual.count==expected.count);assert(!memcmp(actual.events,expected.events,actual.count*sizeof(Event)));
#ifndef DIRECT_STATUS
  assert(actual.reads>=1&&actual.reads<=3841);
#endif
  for(i=0;i<actual.count;i++)if(actual.events[i].service==3)break;timeoutSeen[i<actual.count]++;planSeen[plan]++;modeSeen[!!warm]++;cases++;
 }
 assert(cases==262144);for(i=0;i<8;i++)assert(planSeen[i]);assert(timeoutSeen[0]&&timeoutSeen[1]&&modeSeen[0]&&modeSeen[1]);return 0;
}
'''
  with TemporaryDirectory() as tmp:
   p=Path(tmp);(p/'test.c').write_text(code)
   for mode in [[], ['-DDIRECT_STATUS']]:
    r=subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all',*mode,'-I',str(ROOT/'include'),str(p/'test.c'),'-o',str(p/'test')],capture_output=True,text=True)
    self.assertEqual(r.returncode,0,r.stderr);subprocess.run([str(p/'test')],check=True)
if __name__=='__main__':unittest.main()
