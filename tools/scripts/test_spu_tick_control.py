"""Timer configuration with real callback dispatch/storage and root-counter helpers.

Critical-section and IRQ services are simulated, with mutations between calls.
The counter registers have a complete plain-RAM backing owner; no hardware runs.
"""
from pathlib import Path
from tempfile import TemporaryDirectory
import re, subprocess, unittest
ROOT=Path(__file__).resolve().parents[2]

def native_body(name, rename=None):
 s=(ROOT/f'tools/scripts/fixtures/legacy/func_{name}.c').read_text()
 s=re.sub(r'^#include[^\n]*\n','',s,flags=re.M)
 s=re.sub(r' __asm__\("\$\d+"\)','',s)
 s=re.sub(r'__asm__\(""[^;]*;',';',s)
 if rename:
  import re as _re
  names=['func_'+name]+[m.group(1) for l in open(ROOT/'configs/PAL/sym.main.txt') for m in [_re.match(r'^(\w+)\s*=\s*0x'+name+r';',l,_re.I)] if m]
  for nm in names:s=s.replace(nm,rename)
 return s

class SpuTickControlTest(unittest.TestCase):
 def test_configuration_reloads_native_callbacks_and_unsigned_targets(self):
  helpers='\n'.join(native_body(n,r) for n,r in [('8008B3F4','actual_direct'),('8008BAE8',None),('8008B394',None),('80096FA4','actual_set_counter'),('800970DC','actual_reset_counter')])
  body=native_body('80082A44')
  code=r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <limits.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n)
#define RR_ASSERT_OFFSET(t,f,n)
#include "rr/spu_tick_control.h"
typedef struct {u16 count;u8 pad02[2];u16 mode;u8 pad06[2];u16 target;u8 pad0A[6];} RamCounter;
typedef struct {u32 guard[4];RamCounter records[3];u32 tail[4];} CounterOwner;
typedef struct {u32 guard[4];RrSpuTickCallback values[8];u32 tail[4];} CallbackOwner;
typedef struct {u32 guard[4];RrSpuTickState20 value;u32 tail[4];} TimerOwner;
typedef struct {TimerOwner timer;CounterOwner counters;CallbackOwner callbacks;RrSpuTickCallback irq[11];u32 seed,driver;int critical;} Snapshot;
typedef struct {Snapshot before;u32 code,a,b,c;RrSpuTickCallback callback;} Event;
typedef struct {Snapshot data;Event events[12];u32 count,phase,reference;} Environment;
static Environment actual,expected,*g;
static RrSpuTickInterruptDriver20 drivers[2];
static RrLibSavedState driverStates[2];
static RrSpuTickInterruptDriver20 savedDrivers[2];
static unsigned chosen[2],paths[5],interrupts[11],divided[2];
static void cb0(void){assert(0);}static void cb1(void){assert(0);}
void _SsTickCallback(void){assert(0);}void _SsTickCallbackDivided(void){assert(0);}
static RrSpuTickCallback pool[5]={0,cb0,cb1,_SsTickCallback,_SsTickCallbackDivided};
#define _snd_seq_tick_env (g->data.timer.value)
#define IntrDriverTablePtr (&drivers[g->data.driver].prefix)
#define VSyncCallbackTable (g->data.callbacks.values)
#define RCNT_REGS ((RrRootCounterRegisters *)g->data.counters.records)
static u32 next(u32 *p){u32 n=*p;n^=n<<13;n^=n>>17;n^=n<<5;return *p=n;}
static s32 signed_bits(u32 n){s32 r;memcpy(&r,&n,4);return r;}
static void event(u32 code,u32 a,u32 b,u32 c,RrSpuTickCallback callback){Event *e;assert(g->count<12);e=&g->events[g->count++];e->before=g->data;e->code=code;e->a=a;e->b=b;e->c=c;e->callback=callback;}
static void mutate(u32 service){RrSpuTickState20 *t=&g->data.timer.value;u32 r,i;
 if(!g->phase)return;r=next(&g->data.seed);g->data.driver=r&1;
 t->selector=signed_bits(next(&g->data.seed));t->active=signed_bits(next(&g->data.seed));
 t->callback=pool[next(&g->data.seed)%5];t->previous=pool[next(&g->data.seed)%5];
 t->flags.direct=(s8)next(&g->data.seed);t->flags.divided=(s8)next(&g->data.seed);t->flags.interrupt=(s8)(next(&g->data.seed)%11);t->flags.reserved=(s8)next(&g->data.seed);
 for(i=0;i<3;i++)t->state[i]=signed_bits(next(&g->data.seed));
 if(service==1&&g->phase==2){t->flags.interrupt=0;t->flags.divided=0;}
}
void EnterCriticalSection(void){assert(!g->data.critical);event(1,0,0,0,0);g->data.critical=1;mutate(1);}
void ExitCriticalSection(void){assert(g->data.critical);event(6,0,0,0,0);mutate(6);g->data.critical=0;}
static RrSpuTickCallback exchange(unsigned driver,s32 channel,RrSpuTickCallback callback){RrSpuTickCallback previous;assert(g->data.critical&&channel>=0&&channel<11);event(4,(u32)channel,driver,0,callback);previous=g->data.irq[channel];g->data.irq[channel]=callback;interrupts[channel]++;chosen[driver]++;mutate(4);return previous;}
static RrSpuTickCallback exchange0(s32 n,RrSpuTickCallback c){return exchange(0,n,c);}
static RrSpuTickCallback exchange1(s32 n,RrSpuTickCallback c){return exchange(1,n,c);}
'''+helpers+r'''
static void direct(unsigned driver,s32 slot,RrSpuTickCallback callback){assert(g->data.critical&&slot==4);event(5,(u32)slot,driver,0,callback);chosen[driver]++;
 if(g->reference)g->data.callbacks.values[slot]=callback;else setIntrVSync(slot,callback);
}
static void direct0(s32 n,RrSpuTickCallback c){direct(0,n,c);}static void direct1(s32 n,RrSpuTickCallback c){direct(1,n,c);}
void VSyncCallback(RrSpuTickCallback callback){actual_direct(callback);}
static s32 model_reset(u32 selector){u32 n=selector&65535;if(n>=3)return 0;g->data.counters.records[n].count=0;return 1;}
static s32 model_set(u32 selector,u16 target,s32 mode){u32 n=selector&65535;u16 result=0x48;
 if(n>=3)return 0;
 if(n<2){if(mode&16)result=0x49;if(!(mode&1))result|=0x100;}else if(!(mode&1))result=0x248;
 if(mode&4096)result|=16;g->data.counters.records[n].mode=result;g->data.counters.records[n].target=target;return 1;
}
s32 ResetRCnt(u32 selector){s32 result;assert(g->data.critical);event(2,selector,0,0,0);result=g->reference?model_reset(selector):actual_reset_counter(selector);assert(result==((selector&65535)<3));return result;}
s32 SetRCnt(u32 selector,u16 target,s32 mode){s32 result;assert(g->data.critical);event(3,selector,target,(u32)mode,0);result=g->reference?model_set(selector,target,mode):actual_set_counter(selector,target,mode);assert(result==((selector&65535)<3));return result;}
'''+body+r'''
static void model(s32 argument){RrSpuTickState20 *t=&g->data.timer.value;u32 counter=0xF2000002;u16 period=17640;int direct;s32 index;RrSpuTickCallback callback,old;
 t->flags.interrupt=6;t->flags.direct=0;t->flags.divided=0;t->previous=0;
 if(t->selector==0){t->flags.interrupt=127;paths[0]++;return;}
 if(t->selector==5){t->flags.interrupt=0;if(!argument)t->flags.direct=1;else{counter=0xF2000003;period=1;}}
 else if(t->selector==3)period=35280;
 else if(t->selector!=2){s32 frequency=t->selector;if(t->active){paths[1]++;return;}assert(frequency!=0);if(frequency<70){period=(u16)(2116800/frequency);t->flags.divided=1;paths[2]++;}else{period=(u16)(4233600/frequency);paths[3]++;}}
 else paths[4]++;
 direct=t->flags.direct!=0;EnterCriticalSection();
 if(direct){unsigned selected=g->data.driver;event(5,4,selected,0,t->callback);chosen[selected]++;g->data.callbacks.values[4]=t->callback;}
 else{
  ResetRCnt(counter);SetRCnt(counter,period,4096);index=t->flags.interrupt;
  if(!index){old=exchange(g->data.driver,0,0);t->previous=old;index=t->flags.interrupt;callback=_SsTickCallback;}
  else{divided[t->flags.divided!=0]++;callback=t->flags.divided?_SsTickCallbackDivided:t->callback;}
  exchange(g->data.driver,index,callback);
 }
 ExitCriticalSection();
}
static void initialize(Environment *e,u32 seed,u32 phase,s32 selector,s32 argument){unsigned i;u8 *p;
 memset(e,0,sizeof(*e));e->data.seed=seed;e->phase=phase;e->data.driver=(seed>>1)&1;
 e->data.timer.value.selector=selector;e->data.timer.value.active=(argument&2)?signed_bits(seed|1U):0;
 e->data.timer.value.callback=pool[seed%5];e->data.timer.value.previous=pool[(seed>>4)%5];
 p=(u8*)&e->data.timer.value.flags;for(i=0;i<4;i++)p[i]=(u8)next(&e->data.seed);
 for(i=0;i<3;i++)e->data.timer.value.state[i]=signed_bits(next(&e->data.seed));
 for(i=0;i<4;i++){e->data.timer.guard[i]=next(&e->data.seed);e->data.timer.tail[i]=next(&e->data.seed);e->data.counters.guard[i]=next(&e->data.seed);e->data.counters.tail[i]=next(&e->data.seed);e->data.callbacks.guard[i]=next(&e->data.seed);e->data.callbacks.tail[i]=next(&e->data.seed);}
 p=(u8*)e->data.counters.records;for(i=0;i<sizeof(e->data.counters.records);i++)p[i]=(u8)next(&e->data.seed);
 for(i=0;i<8;i++)e->data.callbacks.values[i]=pool[next(&e->data.seed)%5];for(i=0;i<11;i++)e->data.irq[i]=pool[next(&e->data.seed)%5];
}
int main(void){static const s32 edges[16]={INT32_MIN,INT32_MAX,-1,-70,-69,1,4,6,69,70,71,120,240,0,2,5};u32 seed=0x82A44,n,phase,i;uint64_t cases=0;s32 selector,argument;
 _Static_assert(sizeof(RamCounter)==16&&sizeof(RrRootCounterRegisters)==16,"counter storage");
 _Static_assert(offsetof(RamCounter,mode)==4&&offsetof(RamCounter,target)==8,"counter fields");
 _Static_assert(sizeof(RrSpuTickState20)==24+2*sizeof(void*),"complete native timer");
 _Static_assert(sizeof(RrSpuTickInterruptDriver20)==8*sizeof(void*),"complete native driver");
 _Static_assert(offsetof(RrSpuTickInterruptDriver20,install14)==5*sizeof(void*),"native dispatcher slot");
 _Static_assert(offsetof(RrSpuTickInterruptDriver20,state1C)==7*sizeof(void*),"native driver state pointer");
 _Static_assert(sizeof(unsigned long)>=sizeof(void*),"counter pointer arithmetic");
 assert((uintptr_t)&actual>UINT32_MAX&&(uintptr_t)cb0>UINT32_MAX);
 drivers[0].prefix.install=exchange0;drivers[1].prefix.install=exchange1;drivers[0].install14=direct0;drivers[1].install14=direct1;
 for(i=0;i<2;i++){drivers[i].reserved10=cb0;drivers[i].reserved18=cb1;drivers[i].state1C=&driverStates[i];savedDrivers[i]=drivers[i];}
 for(n=0;n<65536;n++)for(phase=0;phase<8;phase++){
  if(phase<4)selector=(s16)n;else if(phase<6)selector=signed_bits(next(&seed));else selector=edges[n%16];
  argument=(phase==0||phase==6)?0:phase==1?1:phase==2?INT32_MIN:signed_bits(next(&seed));
  initialize(&actual,next(&seed),phase<2?0:phase<4?1:phase<6?2:1,selector,argument);
  if(phase==0||phase==2||phase==4||phase==6)actual.data.timer.value.active=0;
  expected=actual;expected.reference=1;g=&expected;model(argument);g=&actual;_SsStart(argument);
  assert(!memcmp(&actual.data,&expected.data,sizeof(actual.data)));assert(actual.count==expected.count);assert(!memcmp(actual.events,expected.events,actual.count*sizeof(Event)));cases++;
 }
 for(n=0;n<65536;n++){
  initialize(&actual,next(&seed),0,2,0);expected=actual;g=&expected;assert(model_set(2,(u16)n,4096)==1);g=&actual;assert(actual_set_counter(2,(u16)n,4096)==1);assert(!memcmp(&actual.data,&expected.data,sizeof(actual.data)));cases++;
 }
 for(n=0;n<8;n++)for(phase=0;phase<3;phase++){
  initialize(&actual,next(&seed),0,0,0);expected=actual;expected.data.callbacks.values[n]=pool[phase];g=&actual;setIntrVSync((s32)n,pool[phase]);assert(!memcmp(&actual.data,&expected.data,sizeof(actual.data)));cases++;
 }
 assert(!memcmp(drivers,savedDrivers,sizeof(drivers)));
 assert(cases==589848);for(i=0;i<5;i++)assert(paths[i]);for(i=0;i<11;i++)assert(interrupts[i]);assert(chosen[0]&&chosen[1]&&divided[0]&&divided[1]);return 0;
}
'''
  with TemporaryDirectory() as tmp:
   p=Path(tmp);(p/'test.c').write_text(code)
   r=subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-Wno-deprecated-non-prototype','-I',str(ROOT/'include'),str(p/'test.c'),'-o',str(p/'test')],capture_output=True,text=True)
   self.assertEqual(r.returncode,0,r.stderr);subprocess.run([str(p/'test')],check=True)
if __name__=='__main__':unittest.main()
